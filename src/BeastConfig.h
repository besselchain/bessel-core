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

#ifndef BEAST_BEASTCONFIG_H_INCLUDED
#define BEAST_BEASTCONFIG_H_INCLUDED

/** Configuration file for Beast
    This sets various configurable options for Beast. In order to compile you
    must place a copy of this file in a location where your build environment
    can find it, and then customize its contents to suit your needs.
    @file BeastConfig.h
*/

//------------------------------------------------------------------------------
//
// Unit Tests
//
//------------------------------------------------------------------------------

/** Config: BEAST_NO_UNIT_TEST_INLINE
    Prevents unit test definitions from being inserted into a global table.
    The default is to include inline unit test definitions.
*/

#ifndef BEAST_NO_UNIT_TEST_INLINE
//#define BEAST_NO_UNIT_TEST_INLINE 1
#endif

//------------------------------------------------------------------------------
//
// Diagnostics
//
//------------------------------------------------------------------------------

/** Config: BEAST_FORCE_DEBUG
    Normally, BEAST_DEBUG is set to 1 or 0 based on compiler and project
    settings, but if you define this value, you can override this to force it
    to be true or false.
*/
#ifndef   BEAST_FORCE_DEBUG
//#define BEAST_FORCE_DEBUG 1
#endif

/** Config: BEAST_CHECK_MEMORY_LEAKS
    Enables a memory-leak check for certain objects when the app terminates.
*/
#ifndef   BEAST_CHECK_MEMORY_LEAKS
//#define BEAST_CHECK_MEMORY_LEAKS 0
#endif

//------------------------------------------------------------------------------
//
// Libraries
//
//------------------------------------------------------------------------------

/** Config: BEAST_DONT_AUTOLINK_TO_WIN32_LIBRARIES
    In a Visual C++  build, this can be used to stop the required system libs
    being automatically added to the link stage.
*/
#ifndef   BEAST_DONT_AUTOLINK_TO_WIN32_LIBRARIES
//#define BEAST_DONT_AUTOLINK_TO_WIN32_LIBRARIES 1
#endif

/** Config: BEAST_INCLUDE_ZLIB_CODE
    This can be used to disable Beast's embedded 3rd-party zlib code.
    You might need to tweak this if you're linking to an external zlib library
    in your app, but for normal apps, this option should be left alone.

    If you disable this, you might also want to set a value for
    BEAST_ZLIB_INCLUDE_PATH, to specify the path where your zlib headers live.
*/
#ifndef   BEAST_INCLUDE_ZLIB_CODE
//#define BEAST_INCLUDE_ZLIB_CODE 1
#endif

/** Config: BEAST_ZLIB_INCLUDE_PATH
    This is included when BEAST_INCLUDE_ZLIB_CODE is set to zero.
*/
#ifndef BEAST_ZLIB_INCLUDE_PATH
#define BEAST_ZLIB_INCLUDE_PATH <zlib.h>
#endif

/** Config: BEAST_SQLITE_FORCE_NDEBUG
    Setting this option forces sqlite into release mode even if NDEBUG is not set
*/
#ifndef BEAST_SQLITE_FORCE_NDEBUG
#define BEAST_SQLITE_FORCE_NDEBUG 1
#endif

//------------------------------------------------------------------------------
//
// Boost
//
//------------------------------------------------------------------------------

/** Config: BEAST_USE_BOOST_FEATURES
    This activates boost specific features and improvements. If this is
    turned on, the include paths for your build environment must be set
    correctly to find the boost headers.
*/
#ifndef BEAST_USE_BOOST_FEATURES
#define BEAST_USE_BOOST_FEATURES 1
#endif

//------------------------------------------------------------------------------
//
// Bessel
//
//------------------------------------------------------------------------------

/** Config: BESSEL_VERIFY_NODEOBJECT_KEYS
    This verifies that the hash of node objects matches the payload.
    It is quite expensive so normally this is turned off!
*/
#ifndef   BESSEL_VERIFY_NODEOBJECT_KEYS
//#define BESSEL_VERIFY_NODEOBJECT_KEYS 1
#endif

/** Config: BESSEL_DUMP_LEAKS_ON_EXIT
    Displays heap blocks and counted objects which were not disposed of
    during exit.
*/
#ifndef BESSEL_DUMP_LEAKS_ON_EXIT
#define BESSEL_DUMP_LEAKS_ON_EXIT 1
#endif

//------------------------------------------------------------------------------

// These control whether or not certain functionality gets
// compiled into the resulting besseld executable

/** Config: BESSEL_ROCKSDB_AVAILABLE
    Controls whether or not the RocksDB database back-end is compiled into
    besseld. RocksDB requires a relatively modern C++ compiler (tested with
    gcc versions 4.8.1 and later) that supports some C++11 features.
*/
#ifndef   BESSEL_ROCKSDB_AVAILABLE
//#define BESSEL_ROCKSDB_AVAILABLE 0
#endif

//------------------------------------------------------------------------------

// Here temporarily to turn off new Validations code while it
// is being written.
//
#ifndef BESSEL_USE_VALIDATORS
#define BESSEL_USE_VALIDATORS 0
#endif

/** Config: BEAST_USE_BOOST_FEATURES
    This activates boost specific features and improvements. If this is
    turned on, the include paths for your build environment must be set
    correctly to find the boost headers.
*/
#ifndef   BEAST_USE_BOOST_FEATURES
//#define BEAST_USE_BOOST_FEATURES 1
#endif

/** Config: BESSEL_PROPOSE_FEATURES
    This determines whether to add any features to the proposed transaction set.
*/
#ifndef BESSEL_PROPOSE_AMENDMENTS
#define BESSEL_PROPOSE_AMENDMENTS 0
#endif

/** Config: BESSEL_SINGLE_IO_SERVICE_THREAD
    When set, restricts the number of threads calling io_service::run to one.
    This is useful when debugging.
*/
#ifndef BESSEL_SINGLE_IO_SERVICE_THREAD
#define BESSEL_SINGLE_IO_SERVICE_THREAD 0
#endif

/** Config: BESSEL_HOOK_VALIDATORS
    Activates code for handling validations and validators (work in progress).
*/
#ifndef BESSEL_HOOK_VALIDATORS
#define BESSEL_HOOK_VALIDATORS 0
#endif

/** Config: BESSEL_ENABLE_TICKETS
    Enables processing of ticket transactions
*/
#ifndef BESSEL_ENABLE_TICKETS
#define BESSEL_ENABLE_TICKETS 0
#endif

/** Config: USEMYSQL
    use Mysql Database
*/
#ifndef USEMYSQL
#define USEMYSQL
#endif

#endif
