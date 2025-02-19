$!========================================================================
$!
$! Main xpdf compile script for VMS.
$!
$! Copyright 1996 Derek B. Noonburg
$!
$!========================================================================
$!
$ set default [.goo]
$ @vmsdecccomp.com
$ set default [-.ltk]
$ @vmsdecccomp.com
$ set default [-.xpdf]
$ @vmsdecccomp.com
$ set default [-]
