#pragma once
#ifndef STFU_H
#define STFU(x) warning(disable: x)

#pragma STFU(4756) // overflow in constant arithmetic, caused by INFINITY
#pragma STFU(4996) // blahblah may be unsafe
#pragma STFU(4530) // something something xlocale
#endif