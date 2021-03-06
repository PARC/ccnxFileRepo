/*
 * Copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL XEROX OR PARC BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ################################################################################
 * #
 * # PATENT NOTICE
 * #
 * # This software is distributed under the BSD 2-clause License (see LICENSE
 * # file).  This BSD License does not make any patent claims and as such, does
 * # not act as a patent grant.  The purpose of this section is for each contributor
 * # to define their intentions with respect to intellectual property.
 * #
 * # Each contributor to this source code is encouraged to state their patent
 * # claims and licensing mechanisms for any contributions made. At the end of
 * # this section contributors may each make their own statements.  Contributor's
 * # claims and grants only apply to the pieces (source code, programs, text,
 * # media, etc) that they have contributed directly to this software.
 * #
 * # There is no guarantee that this section is complete, up to date or accurate. It
 * # is up to the contributors to maintain their portion of this section and up to
 * # the user of the software to verify any claims herein.
 * #
 * # Do not remove this header notification.  The contents of this section must be
 * # present in all distributions of the software.  You may only modify your own
 * # intellectual property statements.  Please provide contact information.
 *
 * - Palo Alto Research Center, Inc
 * This software distribution does not grant any rights to patents owned by Palo
 * Alto Research Center, Inc (PARC). Rights to these patents are available via
 * various mechanisms. As of January 2016 PARC has committed to FRAND licensing any
 * intellectual property used by its contributions to this software. You may
 * contact PARC at cipo@parc.com for more information or visit http://www.ccnx.org
 */
/**
 * @author Christopher A. Wood, Palo Alto Research Center (Xerox PARC)
 * @copyright (c) 2016, Xerox Corporation (Xerox) and Palo Alto Research Center, Inc (PARC).  All rights reserved.
 */

#ifndef ccnxFileRepoManifestFetcher_h
#define ccnxFileRepoManifestFetcher_h

#include <ccnx/common/ccnx_Name.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

struct ccnx_manifest_fetcher;
typedef struct ccnx_manifest_fetcher CCNxFileRepoManifestFetcher;

/**
 * Create a new `CCNxManifestFetcher` that uses the given portal to recover
 * application data from the given Manifest.
 *
 * @param [in] port The `CCNxPortal` through which to resolve a Manifest.
 * @param [in] root The root of a `CCNxManifest` to resolve.
 *
 * @return A new `CCNxFileRepoManifestFetcher` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ...
 *     CCNxManifest *manifest = ...
 *     CCNxFileRepoManifestFetcher *cache = ccnxFileRepoManifestFetcher_Create(portal, manifest);
 * }
 * @endcode
 */
CCNxFileRepoManifestFetcher *ccnxFileRepoManifestFetcher_Create(CCNxPortal *portal, CCNxManifest *root);

/**
 * Increase the number of references to a `CCNxFileRepoManifestFetcher` instance.
 *
 * Note that new `CCNxFileRepoManifestFetcher` is not created,
 * only that the given `CCNxFileRepoManifestFetcher` reference count is incremented.
 * Discard the reference by invoking `ccnxFileRepoManifestFetcher_Release`.
 *
 * @param [in] instance A pointer to a valid CCNxFileRepoManifestFetcher instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxFileRepoManifestFetcher *a = ccnxFileRepoManifestFetcher_Create();
 *
 *     CCNxFileRepoManifestFetcher *b = ccnxFileRepoManifestFetcher_Acquire(a);
 *
 *     ccnxFileRepoManifestFetcher_Release(&a);
 *     ccnxFileRepoManifestFetcher_Release(&b);
 * }
 * @endcode
 */
 CCNxFileRepoManifestFetcher *ccnxFileRepoManifestFetcher_Acquire(const CCNxFileRepoManifestFetcher *instance);

/**
 * Release a previously acquired reference to the given `CCNxFileRepoManifestFetcher` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxFileRepoManifestFetcher *a = ccnxFileRepoManifestFetcher_Create(...);
 *
 *     ccnxFileRepoManifestFetcher_Release(&a);
 * }
 * @endcode
 */
void ccnxFileRepoManifestFetcher_Release(CCNxFileRepoManifestFetcher **instancePtr);

/**
 * Fill the provided `PARCBuffer` with application data. Return false if more data
 * exists in the Manifest.
 *
 * @param [in] fetcher A `CCNxManifestFetcher` instance.
 * @param [in, out] buffer A `PARCBuffer` to fill with application data bytes.
 *
 * Example:
 * @code
 * {
 *     CCNxFileRepoManifestFetcher *fetcher = ...
 *     PARCBuffer *buffer = parcBuffer_Allocate(8192);
 *
 *     bool done = ccnxFileRepoManifestFetcher_FillBuffer(fetcher, buffer);
 *     while (!done) {
 *         // use the data and then fill more into the buffer
 *     }
 * }
 * @endcode
 */
bool ccnxFileRepoManifestFetcher_FillBuffer(CCNxFileRepoManifestFetcher *fetcher, PARCBuffer *buffer);
#endif // ccnxFileRepoManifestFetcher_h
