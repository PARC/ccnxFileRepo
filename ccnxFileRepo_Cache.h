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

#ifndef ccnxFileRepoCache_h
#define ccnxFileRepoCache_h

#include <stdint.h>

struct ccnx_file_repo_cache;
typedef struct ccnx_file_repo_cache CCNxFileRepoCache;

/**
 * Create a `CCNxFileRepoCache` instance that stores chunks in the specified
 * directory. Each chunk will be of the specified size.
 *
 * @param [in] directory The output chunk directory.
 * @param [in] chunkSize Chunk size for each entry in the repo.
 *
 * @return A new `CCNxFileRepoCache` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxFileRepoCache *cache = ccnxFileRepoCache_Create(".", 4096);
 * }
 * @endcode
 */
CCNxFileRepoCache *ccnxFileRepoCache_Create(char *directory, size_t chunkSize);

/**
 * Increase the number of references to a `CCNxFileRepoCache` instance.
 *
 * Note that new `CCNxFileRepoCache` is not created,
 * only that the given `CCNxFileRepoCache` reference count is incremented.
 * Discard the reference by invoking `ccnxFileRepoCache_Release`.
 *
 * @param [in] instance A pointer to a valid CCNxFileRepoCache instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxFileRepoCache *a = ccnxFileRepoCache_Create();
 *
 *     CCNxFileRepoCache *b = ccnxFileRepoCache_Acquire(a);
 *
 *     ccnxFileRepoCache_Release(&a);
 *     ccnxFileRepoCache_Release(&b);
 * }
 * @endcode
 */
 CCNxFileRepoCache *ccnxFileRepoCache_Acquire(const CCNxFileRepoCache *instance);

/**
 * Release a previously acquired reference to the given `CCNxFileRepoCache` instance,
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
 *     CCNxFileRepoCache *a = ccnxFileRepoCache_Create();
 *
 *     ccnxFileRepoCache_Release(&a);
 * }
 * @endcode
 */
void ccnxFileRepoCache_Release(CCNxFileRepoCache **instancePtr);

/**
 * Search for a file (Manifest or Content Object chunk) in the cache by its
 * ContentObjectHashRestriction digest.
 *
 * @param [in] repo The `CCNxFileRepoCache` instance.
 * @param [in] digest The hash digest of the chunk being sought after.
 *
 * @retval NULL The file was not found.
 * @retval PARCBuffer A `PARCBuffer` containing the file contents.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *contentObjectHash = <hash from manifest>
 *     PARCBuffer *ccnxFileRepoCache_SearchForFile(CCNxFileRepoCache *repo, contentObjectHash);
 *     // use the buffer
 * }
 * @endcode
 */
PARCBuffer *ccnxFileRepoCache_CreateWireEncodedMessageWithDigest(CCNxFileRepoCache *repo, PARCBuffer *fileName);

/**
 * Load the specified file into the repository and give each chunk the specified name.
 *
 * @param [in] repo The `CCNxFileRepoCache` instance.
 * @param [in] name The `CCNxName` for each chunk.
 * @param [in] file A `PARCFile` pointing to the file to load into the repository.
 *
 * @retval NULL The file could not be loaded.
 * @retval CCNxManifest The root `CCNxManifest` in the for the file.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("some_file.bin");
 *     CCNxName *dataName = ccnxName_CreateFromCString("ccnx:/some/file");
 *
 *     CCNxManifest *ccnxFileRepoCache_LoadFile(CCNxFileRepoCache *repo, dataName, file);
 * }
 * @endcode
 */
CCNxManifest *ccnxFileRepoCache_LoadFile(CCNxFileRepoCache *cache, CCNxName *name, PARCFile *file);
#endif // ccnxFileRepoCache_h
