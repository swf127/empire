.TH Command NUKE
.NA nuke "Report status of nukes"
.LV Expert
.SY "nuke <NUKES>"
The nuke report command is a census of your nukes.  For example:
.EX nuke *
.NF
   # nuke type              x,y    s   eff tech carry burst
   0 10kt  fission        -24,-4      100%  296   19P   air
   2 15kt  fission        -19,-1      100%  296
2 nukes
.FI
.s1
The report format contains the following fields:
.s1
.in \w'nuke type\0\0'u
.L #
the nuke number
.L "nuke type"
the type of nuke; \*Q10kt fission\*U, \*Q1mt fusion\*U, etc.,
.L x,y
the nuke's current location,
.L s
the \*Qstockpile\*U designation letter,
.L eff
the nuke's efficiency, prefixed by \*Q=\*U if stopped,
.L tech
the tech level at which it was created,
.L carry
the plane the nuke is on, if any,
.L burst
whether the nuke is programmed for air or ground burst.
.in
.s1
.SA "arm, build, launch, transport, Planes, Nukes"
