#!/bin/sh

echo "Reconfiguring project  "

#
# originalment:
# aclocal; autoheader; autoconf; automake --add-missing
if [ "$1" = "special" ]; then
  AUTOCONF_HOME=/usr/local/share/autoconf-2.59
  AUTOHEADER_HOME=/usr/local/share/autoconf-2.59
  AUTOMAKE_HOME=/usr/local/share/automake-1.8.2
  ACLOCAL_HOME=/usr/local/share/automake-1.8.2
elif [ -z "$1" ]; then
  AUTOCONF_HOME=/usr/
  AUTOHEADER_HOME=/usr/bin/
  AUTOMAKE_HOME=/usr/bin/
  ACLOCAL_HOME=/usr/bin/
else
  echo "Incorrect parameter"
fi
# La instalacio esta feta amb el cul!
# Hi ha maquines que tenen alguns binaris sota el directori a sac
# del paquet!

if [ -e ${AUTOMAKE_HOME}bin/automake ]; then
AUTOMAKE=${AUTOMAKE_HOME}bin/automake
ACLOCAL=${ACLOCAL_HOME}bin/aclocal
else
AUTOMAKE=${AUTOMAKE_HOME}automake
ACLOCAL=${ACLOCAL_HOME}aclocal
fi

AUTOHEADER=${AUTOHEADER_HOME}bin/autoheader
AUTOCONF=${AUTOCONF_HOME}bin/autoconf
AUTOM4TE=${AUTOCONF_HOME}bin/autom4te
export AUTOCONF
export AUTOM4TE

#
# Suport instala les coses sense criteri!
# A vegades posen binaris sota el directori BIN i a vegades ho foten
# sota el directory del package... :\
#

echo 'Running "aclocal"'
${ACLOCAL} 
#echo 'Running "autoheader"'
#${AUTOHEADER}
echo 'Running "autoconf"'
${AUTOMAKE} --add-missing --copy
echo 'Running "automake"'
${AUTOCONF}

echo "DONE"
