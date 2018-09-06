.TH Command SUPPLY
.NA supply "Cause a unit to draw supplies"
.LV Basic
.SY "supply <UNIT/ARMY>"
The supply command causes a unit to draw enough supplies from its
supply sources to make sure that it has its basic load of supplies.
Supply tells you which units have enough supplies.
.s1
A basic load of supplies for a unit consists of:
.nf
	1) enough food to eat for one update

	2) enough shells to fire once
		A unit uses shells equal to its ammo stat to fire once.
		Use the \*Qshow\*U command to find out how much ammo
		the unit uses per shot.
.fi

.s1
If the unit has enough of all these things, it is said to be 'in supply'.
A lack of any of these things is enough to make the unit 'out of supply'.
.s1
.L "When supplies are drawn & used"
\" Has been disabled for ages:
\" A unit that fights may use a basic load of supplies
\" to attack with.  (depending on the intensity of combat. The more intense the
\" combat, the more chance the unit will use a basic load of supplies)
.s1
When a unit needs to draw supplies, such as when you use the supply command,
or the unit wishes to attack, or is attacked, etc, it must attempt to get
them from supply sources. Other things may use the same routines, such as
ships/forts/sectors, which will draw shells when they need them to
fire defensively.
.s1
.L "Supply Sources"
.s1
There are four sources of supply. In each case, the
unit must be within supply range of the source. Supply range is:
.s1
.ti 3
10 * ((tech+50)/(tech+200) sectors.
.s1
In addition there must be a valid supply path to the source. A supply path
is a path of sectors owned by the owner of the unit that leads to the
supply source. (Note that the path may be of any length.. only the supply
source itself must be within supply range.. this is to make the coding
simpler, and may change in the future) The best path (in terms of mobility
use) will be used.
.s1
In each case, mobility is used to move the supplies.
.s1
The first source of supply checked is the sector the unit is in.
.s1
If the unit's needs can't be satisfied there, it looks for an owned
harbor, warehouse or headquarters that is at least 60% efficient. Drawing
from these sources uses mobility from
the source sector. (A headquarters will NOT supply a unit if it
has no distribution path)
.s1
If still in need, the unit will look for an owned supply ship (one that has
the supply ability) in an owned harbor.
The harbor must be at least 2%
efficient. This uses mobility from the harbor.
.s1
If it still can't find enough supplies, the unit will look for an owned
unit with the 'supply' ability. (see info show for information on how to
find out about abilities of units) In this case, the supply unit uses its
mobility to use the supplies.
\" Disabled due to bugs:
\" After it supplies the unit, the supply unit
\" will then itself try to re-supply, so that it is possible to form a chain
\" of supply units reaching back to a headquarters or supply ship.
.s1
If after all this the unit can't get enough, it is out of supplies.
.s1
Note that no supply source will give up enough food to make it starve. In
addition, land units will keep enough shells to fire once.
.s1
.EX supply *
War band #2 has supplies
.ti 3
4 units
.s1
.s1
.SA "lload, LandUnits, Attacking, Mobility"
