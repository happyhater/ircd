#!/bin/sh

spitshell=cat
package=IRC

echo "Extracting $package/src/version.c..."

if test -r version.c.last
then
   generation=`sed -n 's/^char \*generation = \"\(.*\)\";/\1/p' < version.c.last`
   if test ! "$generation" ; then generation=0; fi
else
   generation=0
fi

generation=`expr $generation + 1`

uname=`uname -a`

creation=`LC_ALL=C date | \
awk '{if (NF == 6) \
         { print $1 " "  $2 " " $3 " "  $6 " at " $4 " " $5 } \
else \
         { print $1 " "  $2 " " $3 " " $7 " at " $4 " " $5 " " $6 }}'`

$spitshell >version.c <<!SUB!THIS!
#include "patchlevel.h"
#include "serno.h"

const char *generation = "$generation";
const char *creation = "$creation";
const char *platform = "$uname";
const char *ircd_version = PATCHLEVEL;
const char *charybdis_version = CHARYBDIS_VERSION;
const char *serno = SERNO;

const char *infotext[] =
{
  "$package --",
  "Based on the original code written by Jarkko Oikarinen",
  "Copyright 1988, 1989, 1990, 1991 University of Oulu, Computing Center",
  "Copyright (c) 2018 ZmEu - WhiteHat Development Team (www.whitehat.ro)",
  "",
  "This program is free software; you can redistribute it and/or",
  "modify it under the terms of the GNU General Public License as",
  "published by the Free Software Foundation; either version 2, or",
  "(at your option) any later version.",
  "",
!SUB!THIS!

IFS='
'
$spitshell >>version.c <<!SUB!THISTOO!
  "",
  0,
};
!SUB!THISTOO!
