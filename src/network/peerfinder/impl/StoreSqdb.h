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

#ifndef BESSEL_PEERFINDER_STORESQDB_H_INCLUDED
#define BESSEL_PEERFINDER_STORESQDB_H_INCLUDED

#include <data/database/SociDB.h>
#include <boost/optional.hpp>
#include <boost/asio.hpp>
#include <common/misc/Utility.h>

namespace bessel {
namespace PeerFinder {

/** Database persistence for PeerFinder using SQLite */
class StoreSqdb
    : public Store
{
private:
    beast::Journal m_journal;
    soci::session m_session;

public:
    enum
    {
        // This determines the on-database format of the data
        currentSchemaVersion = 4
    };

    explicit StoreSqdb (beast::Journal journal = beast::Journal())
        : m_journal (journal)
    {
    }

    ~StoreSqdb ()
    {
    }

    void open (SociConfig const& sociConfig)
    {
        sociConfig.open (m_session);

        m_journal.info << "Opening database at '" 
                       << sociConfig.connectionString ()
                       << "'";

        init ();

        update ();
    }

    // Loads the bootstrap cache, calling the callback for each entry
    //
    std::size_t load (load_callback const& cb)
    {
        std::size_t n (0);
        std::string s;
        int valence;

        soci::statement st = (m_session.prepare <<
            "SELECT "
            " address, "
            " valence "
            "FROM PeerFinder_BootstrapCache;"
            , soci::into (s)
            , soci::into (valence)
            );

        st.execute ();

        while (st.fetch ())
        {
            std::string str(s);
            boost::asio::ip::tcp::endpoint const endpoint (endpoint_from_string(str).first);

            if (!endpoint.address().is_unspecified ())
            {
                cb (endpoint, valence);
                ++n;
            }
            else
            {
                m_journal.error << "Bad address string '" << s << "' in Bootcache table";
            }
        }

        return n;
    }

    // Overwrites the stored bootstrap cache with the specified array.
    //
    void save (std::vector <Entry> const& v)
    {
        soci::transaction tr (m_session);
        m_session << "DELETE FROM PeerFinder_BootstrapCache;";

        if (!v.empty ())
        {
            std::vector<std::string> s;
            std::vector<int> valence;
            s.reserve (v.size ());
            valence.reserve (v.size ());

            for (auto const& e : v)
            {
                s.emplace_back (convert_endpoint_to_string(e.endpoint));
                valence.emplace_back (e.valence);
            }

            m_session <<
                    "INSERT INTO PeerFinder_BootstrapCache ( "
                    "  address, "
                    "  valence "
                    ") VALUES ( "
                    "  :s, :valence "
                    ");"
                    , soci::use (s)
                    , soci::use (valence);
        }

        tr.commit ();
    }

    // Convert any existing entries from an older schema to the
    // current one, if appropriate.
    void update ()
    {
        soci::transaction tr (m_session);

        // get version
        int version (0);
        {
            boost::optional<int> vO;
            m_session <<
                "SELECT "
                "  version "
                "FROM SchemaVersion WHERE "
                "  name = 'PeerFinder';"
                , soci::into (vO)
                ;

            version = vO.value_or (0);

            m_journal.info << "Opened version " << version << " database";
        }

        {
            if (version < currentSchemaVersion)
            {
                m_journal.info << "Updating database to version " << currentSchemaVersion;
            }
            else if (version > currentSchemaVersion)
            {
                throw std::runtime_error ("The PeerFinder database version is higher than expected");
            }
        }

        if (version < 4)
        {
            //
            // Remove the "uptime" column from the bootstrap table
            //

            m_session <<
                "CREATE TABLE IF NOT EXISTS PeerFinder_BootstrapCache_Next ( "
                "  id       INTEGER PRIMARY KEY AUTO_INCREMENT, "
				"  address  VARCHAR(255) UNIQUE NOT NULL, "
                "  valence  INTEGER"
                ");"
                ;


   //          // TODO index
			// try{
			// 	m_session <<
			// 		"CREATE INDEX "
			// 		"  PeerFinder_BootstrapCache_Next_Index ON "
			// 		"    PeerFinder_BootstrapCache_Next "
			// 		"  ( address ); "
			// 		;
			// }
			// catch (...)
			// {
			// 	//ignore if duplicate index is there already
			// }


            std::size_t count;
            m_session <<
                "SELECT COUNT(*) FROM PeerFinder_BootstrapCache;"
                , soci::into (count)
                ;

            std::vector <Store::Entry> list;

            {
                list.reserve (count);
                std::string s;
                int valence;
                soci::statement st = (m_session.prepare <<
                    "SELECT "
                    " address, "
                    " valence "
                    "FROM PeerFinder_BootstrapCache;"
                    , soci::into (s)
                    , soci::into (valence)
                    );

                st.execute ();
                while (st.fetch ())
                {
                    Store::Entry entry;
                    std::string str(s);
                    entry.endpoint = endpoint_from_string (str).first;
                    if (! entry.endpoint.address().is_unspecified ())
                    {
                        entry.valence = valence;
                        list.push_back (entry);
                    }
                    else
                    {
                        m_journal.error << "Bad address string '" << s << "' in Bootcache table";
                    }
                }
            }

            if (!list.empty ())
            {
                std::vector<std::string> s;
                std::vector<int> valence;
                s.reserve (list.size ());
                valence.reserve (list.size ());

                for (auto iter (list.cbegin ());
                     iter != list.cend (); ++iter)
                {
                    s.emplace_back (convert_endpoint_to_string(iter->endpoint));

                    valence.emplace_back (iter->valence);
                }

                m_session <<
                    "INSERT INTO PeerFinder_BootstrapCache_Next ( "
                    "  address, "
                    "  valence "
                    ") VALUES ( "
                    "  :s, :valence"
                    ");"
                    , soci::use (s)
                    , soci::use (valence)
                    ;

            }

            m_session << "DROP TABLE IF EXISTS PeerFinder_BootstrapCache;";

            //m_session <<
            //    "Alter TABLE PeerFinder_BootstrapCache DROP INDEX  PeerFinder_BootstrapCache_Index;";

            m_session <<
                "ALTER TABLE PeerFinder_BootstrapCache_Next "
                "  RENAME TO PeerFinder_BootstrapCache;";

            // TODO index
			// try{
			// 	m_session <<
			// 		"CREATE INDEX  "
			// 		"  PeerFinder_BootstrapCache_Index ON "
			// 		"PeerFinder_BootstrapCache "
			// 		"  (  "
			// 		"    address "
			// 		"  ); "
			// 		;
			// }
			// catch (...){}
        }

        if (version < 3)
        {
            //
            // Remove legacy endpoints from the schema
            //

            m_session <<
                "DROP TABLE IF EXISTS LegacyEndpoints;";

            m_session <<
                "DROP TABLE IF EXISTS PeerFinderLegacyEndpoints;";

            m_session <<
                "DROP TABLE IF EXISTS PeerFinder_LegacyEndpoints;";

            m_session <<
                "DROP TABLE IF EXISTS PeerFinder_LegacyEndpoints_Index;";
        }

        {
            int const version (currentSchemaVersion);
            m_session <<
                "REPLACE INTO SchemaVersion ("
                "   name "
                "  ,version "
                ") VALUES ( "
                "  'PeerFinder', :version "
                ");"
                , soci::use (version);
        }

        tr.commit ();
    }

private:
    void init ()
    {
        soci::transaction tr (m_session);
        //m_session << "PRAGMA encoding=\"UTF-8\";";

        m_session <<
            "CREATE TABLE IF NOT EXISTS SchemaVersion ( "
			"  name             VARCHAR(255) PRIMARY KEY, "
            "  version          INTEGER"
            ");"
            ;

        m_session <<
            "CREATE TABLE IF NOT EXISTS PeerFinder_BootstrapCache ( "
            "  id       INTEGER PRIMARY KEY AUTO_INCREMENT, "
			"  address  VARCHAR(255) UNIQUE NOT NULL, "
            "  valence  INTEGER"
            ");"
            ;

        // TODO index
		// try{
		// 	m_session <<
		// 		"CREATE INDEX "
		// 		"  PeerFinder_BootstrapCache_Index ON "
		// 		"PeerFinder_BootstrapCache "
		// 		"  (  "
		// 		"    address "
		// 		"  ); "
		// 		;
		// }
		// catch (...){}

        tr.commit ();
    }
};

}
}

#endif
