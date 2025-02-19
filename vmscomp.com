$!========================================================================
$!
$! Main xpdf compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$ set default [.goo]
$ @vmscomp.com
$ set default [-.ltk]
$ @vmscomp.com
$ set default [-.xpdf]
$ @vmscomp.com
$ set default [-]
