/*******************************************************************************
MIT License

Copyright (c) 2021 Zhihong Li <humble_zh@163.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/
#ifndef __L_H__
#define __L_H__
#include <stdio.h>
#include <syslog.h>

#define l_e(fmt, arg...) do\
{\
    fprintf(stderr, "%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## arg);\
    syslog(LOG_ERR, "%s:%d: " fmt, __FUNCTION__, __LINE__, ## arg);\
} while (0)

#define l_w(fmt, arg...) do\
{\
    printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## arg);\
    syslog(LOG_WARNING, "%s:%d: " fmt, __FUNCTION__, __LINE__, ## arg);\
} while (0)

#define l_n(fmt, arg...) do\
{\
    printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## arg);\
    syslog(LOG_NOTICE, "%s:%d: " fmt, __FUNCTION__, __LINE__, ## arg);\
} while (0)

#define l_i(fmt, arg...) do\
{\
    printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## arg);\
    syslog(LOG_INFO, "%s:%d: " fmt, __FUNCTION__, __LINE__, ## arg);\
} while (0)

#define l_d(fmt, arg...) do\
{\
    printf("%s:%d: " fmt "\n", __FUNCTION__, __LINE__, ## arg);\
    syslog(LOG_DEBUG, "%s:%d: " fmt, __FUNCTION__, __LINE__, ## arg);\
} while (0)

#endif /* ifndef __L_H__ */
