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

#ifndef BESSEL_MODULE_CORE_DIAGNOSTIC_SEMANTICVERSION_H_INCLUDED
#define BESSEL_MODULE_CORE_DIAGNOSTIC_SEMANTICVERSION_H_INCLUDED

#include <vector>
#include <string>

namespace bessel {

/** A Semantic Version number.

    Identifies the build of a particular version of software using
    the Semantic Versioning Specification described here:

    http://semver.org/
*/
class SemanticVersion
{
public:
    typedef std::vector<std::string> identifier_list;

    int majorVersion;
    int minorVersion;
    int patchVersion;

    identifier_list preReleaseIdentifiers;
    identifier_list metaData;

    SemanticVersion ();

    SemanticVersion (std::string const& version);

    /** Parse a semantic version string.
        The parsing is as strict as possible.
        @return `true` if the string was parsed.
    */
    bool parse (std::string const& input, bool debug = false);

    /** Produce a string from semantic version components. */
    std::string print () const;

    inline bool isRelease () const noexcept
    { 
        return preReleaseIdentifiers.empty();
    }
    inline bool isPreRelease () const noexcept
    {
        return !isRelease ();
    }
};

/** Compare two SemanticVersions against each other.
    The comparison follows the rules as per the specification.
*/
int compare (SemanticVersion const& lhs, SemanticVersion const& rhs);

inline bool
operator== (SemanticVersion const& lhs, SemanticVersion const& rhs) 
{ 
    return compare (lhs, rhs) == 0;
}

inline bool
operator!= (SemanticVersion const& lhs, SemanticVersion const& rhs)
{
    return compare (lhs, rhs) != 0;
}

inline bool
operator>= (SemanticVersion const& lhs, SemanticVersion const& rhs)
{
    return compare (lhs, rhs) >= 0;
}

inline bool
operator<= (SemanticVersion const& lhs, SemanticVersion const& rhs)
{
    return compare (lhs, rhs) <= 0;
}

inline bool
operator>  (SemanticVersion const& lhs, SemanticVersion const& rhs)
{
    return compare (lhs, rhs) >  0;
}

inline bool
operator<  (SemanticVersion const& lhs, SemanticVersion const& rhs)
{
    return compare (lhs, rhs) <  0;
}

} // bessel

#endif
