//------------------------------------------------------------------------------
//*
    This file is part of Bessel Chain Project: https://github.com/Besselfoundation/bessel-core
    Copyright (c) 2018 BESSEL.

    Permission to use, copy, modify, and/or distribute this software for any
    purpose  with  or without fee is hereby granted, provided that the above
    copyright notice and this permission notice appear in all copies.

    THE  SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
    WITH  REGARD  TO  THIS  SOFTWARE  INCLUDING  ALL  IMPLIED  WARRANTIES  OF
    MERCHANTABILITY  AND  FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
    ANY  SPECIAL ,  DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
    WHATSOEVER  RESULTING  FROM  LOSS  OF USE, DATA OR PROFITS, WHETHER IN AN
    ACTION  OF  CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/
//==============================================================================

#include <beast/module/core/files/File.h>
#include <beast/module/core/files/FileInputStream.h>
#include <beast/module/core/files/FileOutputStream.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <beast/module/core/files/DirectoryIterator.h>
#include <sys/statfs.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <elf.h>

#include <dlfcn.h>
#include <stdio.h>
#include <dirent.h>
#include <fnmatch.h>
#include <beast/module/core/native/posix_SharedCode.h>

namespace beast
{

enum
{
    U_ISOFS_SUPER_MAGIC = 5,
    U_MSDOS_SUPER_MAGIC = 2,
    U_NFS_SUPER_MAGIC = 1,
    U_SMB_SUPER_MAGIC = 8
};

//==============================================================================
bool File::copyInternal (const File& dest) const
{
    FileInputStream in (*this);

    if (dest.deleteFile())
    {
        {
            FileOutputStream out (dest);

            if (out.failedToOpen())
                return false;

            if (out.writeFromInputStream (in, -1) == getSize())
                return true;
        }

        dest.deleteFile();
    }

    return false;
}

//==============================================================================
static File resolveXDGFolder (const char* const type, const char* const fallbackFolder)
{
    File userDirs ("~/.config/user-dirs.dirs");
    StringArray confLines;

    if (userDirs.existsAsFile())
    {
        FileInputStream in (userDirs);

        if (in.openedOk())
            confLines.addLines (in.readEntireStreamAsString());
    }

    for (int i = 0; i < confLines.size(); ++i)
    {
        const String line (confLines[i].trimStart());

        if (line.startsWith (type))
        {
            // eg. resolve XDG_MUSIC_DIR="$HOME/Music" to /home/user/Music
            const File f (line.replace ("$HOME", File ("~").getFullPathName())
                              .fromFirstOccurrenceOf ("=", false, false)
                              .trim().unquoted());

            if (f.isDirectory())
                return f;
        }
    }

    return File (fallbackFolder);
}

const char* const* beast_argv = nullptr;
int beast_argc = 0;

File File::getSpecialLocation (const SpecialLocationType type)
{
    switch (type)
    {
        case userHomeDirectory:
        {
            const char* homeDir = getenv ("HOME");

            if (homeDir == nullptr)
            {
                struct passwd* const pw = getpwuid (getuid());
                if (pw != nullptr)
                    homeDir = pw->pw_dir;
            }

            return File (CharPointer_UTF8 (homeDir));
        }

        case userDocumentsDirectory:          return resolveXDGFolder ("XDG_DOCUMENTS_DIR", "~");
        case userMusicDirectory:              return resolveXDGFolder ("XDG_MUSIC_DIR",     "~");
        case userMoviesDirectory:             return resolveXDGFolder ("XDG_VIDEOS_DIR",    "~");
        case userPicturesDirectory:           return resolveXDGFolder ("XDG_PICTURES_DIR",  "~");
        case userDesktopDirectory:            return resolveXDGFolder ("XDG_DESKTOP_DIR",   "~/Desktop");
        case userApplicationDataDirectory:    return File ("~");
        case commonApplicationDataDirectory:  return File ("/var");
        case globalApplicationsDirectory:     return File ("/usr");

        case tempDirectory:
        {
            File tmp ("/var/tmp");

            if (! tmp.isDirectory())
            {
                tmp = "/tmp";

                if (! tmp.isDirectory())
                    tmp = File::getCurrentWorkingDirectory();
            }

            return tmp;
        }

        default:
            bassertfalse; // unknown type?
            break;
    }

    return File::nonexistent ();
}

//==============================================================================
class DirectoryIterator::NativeIterator::Pimpl
{
public:
    Pimpl (const File& directory, const String& wildCard_)
        : parentDir (File::addTrailingSeparator (directory.getFullPathName())),
          wildCard (wildCard_),
          dir (opendir (directory.getFullPathName().toUTF8()))
    {
    }

    Pimpl (Pimpl const&) = delete;
    Pimpl& operator= (Pimpl const&) = delete;

    ~Pimpl()
    {
        if (dir != nullptr)
            closedir (dir);
    }

    bool next (String& filenameFound,
               bool* const isDir, bool* const isHidden, std::int64_t* const fileSize,
               Time* const modTime, Time* const creationTime, bool* const isReadOnly)
    {
        if (dir != nullptr)
        {
            const char* wildcardUTF8 = nullptr;

            for (;;)
            {
                struct dirent* const de = readdir (dir);

                if (de == nullptr)
                    break;

                if (wildcardUTF8 == nullptr)
                    wildcardUTF8 = wildCard.toUTF8();

                if (fnmatch (wildcardUTF8, de->d_name, FNM_CASEFOLD) == 0)
                {
                    filenameFound = CharPointer_UTF8 (de->d_name);

                    updateStatInfoForFile (parentDir + filenameFound, isDir, fileSize, modTime, creationTime, isReadOnly);

                    if (isHidden != nullptr)
                        *isHidden = filenameFound.startsWithChar ('.');

                    return true;
                }
            }
        }

        return false;
    }

private:
    String parentDir, wildCard;
    DIR* dir;
};

DirectoryIterator::NativeIterator::NativeIterator (const File& directory, const String& wildCard)
    : pimpl (new DirectoryIterator::NativeIterator::Pimpl (directory, wildCard))
{
}

DirectoryIterator::NativeIterator::~NativeIterator()
{
}

bool DirectoryIterator::NativeIterator::next (String& filenameFound,
                                              bool* const isDir, bool* const isHidden, std::int64_t* const fileSize,
                                              Time* const modTime, Time* const creationTime, bool* const isReadOnly)
{
    return pimpl->next (filenameFound, isDir, isHidden, fileSize, modTime, creationTime, isReadOnly);
}

} // beast
