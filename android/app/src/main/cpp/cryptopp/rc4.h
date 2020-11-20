/*
 * Copyright 1995-2016 The OpenSSL Project Authors. All Rights Reserved.
 *
 * Licensed under the OpenSSL license (the "License").  You may not use
 * this file except in compliance with the License.  You can obtain a copy
 * in the file LICENSE in the source distribution or at
 * https://www.openssl.org/source/license.html
 */
#pragma once
# include <stddef.h>

//网络包只加密前面128字节
#define maxrc4(l) (l>256?256:l)

namespace rc4
{
	void encrypt(const char* data, int len, const char* key, int keylen);
}





