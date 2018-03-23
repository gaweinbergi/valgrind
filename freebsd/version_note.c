/*-
 * Copyright 2000 David E. O'Brien, John D. Polstra.
 * All rights reserved.
 *
 * Copyright 2018 Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Special ".note" entry specifying the ABI version.  See
 * http://www.netbsd.org/Documentation/kernel/elf-notes.html
 * for more information.
 */
#include <sys/types.h>
#include <sys/param.h>

#define NOTE_FREEBSD_VENDOR	"FreeBSD"
#define ABI_NOTETYPE		1

static const struct {
	int32_t	namesz;
	int32_t	descsz;
	int32_t	type;
	char	name[sizeof(NOTE_FREEBSD_VENDOR)];
	int32_t	desc;
} abitag __attribute__ ((section (".note.tag"), aligned(4))) __used = {
	.namesz = sizeof(NOTE_FREEBSD_VENDOR),
	.descsz = sizeof(int32_t),
	.type = ABI_NOTETYPE,
	.name = NOTE_FREEBSD_VENDOR,
	.desc = __FreeBSD_version
};
