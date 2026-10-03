# Forever Shaman

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module for shaman changes on a
3.3.5 server: Ghost Wolf keeps up with level-scaled mounts, can mine and works indoors. Mining
and casting Ghost Wolf indoors need a small optional client patch; the speed change doesn't.

## Ghost Wolf speed

[mod-mount-scaling](https://github.com/buildthehomelab/wow-mod-mount-scaling) makes mounts faster
as you level, but only changes mount auras, so Ghost Wolf stayed at a flat 40%. With both modules
installed, Ghost Wolf runs as fast as a ground mount would:

| Level / riding | Stock Ghost Wolf | Ground mount and Ghost Wolf |
|---|---|---|
| 20, Apprentice | 40% | 50% |
| 30, Apprentice | 40% | 75% |
| 40, Journeyman | 40% | 100% |
| 60+, Journeyman | 40% | 150% |

The numbers come from mod-mount-scaling's own `MountScaling.*` settings, so the two always match.
A shaman without Apprentice Riding keeps the stock 40%. The speed changes as soon as you shift,
and when you level up in wolf form.

Ghost Wolf only gets the mount speed **out of combat**. Entering combat drops it to the stock 40%
and leaving combat brings it back, so it stays a way to travel rather than a way to kite (a mount
can't be used in combat at all). `ForeverShaman.GhostWolfSpeed.OutOfCombatOnly = 0` keeps the
mount speed in combat too.

Ghost Wolf can still be cast in combat, as in stock, and works indoors (see below). Mount-only
bonuses (Riding Crop, Mithril Spurs, Carrot on a Stick) don't apply to it, and other run speed
effects don't stack with it: the core takes the highest one. Improved Ghost Wolf and the Glyph
of Ghost Wolf work as before.

Without mod-mount-scaling (or with `MountScaling.Enable = 0`) nothing changes.
[mod-forever-druid](https://github.com/buildthehomelab/wow-mod-forever-druid) does the same for
Travel Form and the flight forms.

## Mining in Ghost Wolf

Stock 3.3.5 lets shamans gather herbs and skin in Ghost Wolf, but Mining says "Can't do that while
shapeshifted". With this module every rank of Mining works in Ghost Wolf, and so do mining a
creature's corpse and Engineering salvage. You stay in wolf form.

The client checks this before it asks the server, so it needs the client patch below: without it
the client still refuses. [mod-forever-druid](https://github.com/buildthehomelab/wow-mod-forever-druid)
does the same for the druid forms; the two patch scripts can run on the same Spell.dbc in either
order.

## Ghost Wolf indoors

Stock Ghost Wolf is outdoors only: you can't shift inside a building, cave or dungeon, and walking
into one drops you out of wolf form. With this module you keep Ghost Wolf when you walk inside, and
can cast it there. Speed indoors is the same as outdoors.

Keeping it when you walk in works without the client patch. Casting it indoors needs the patch
below, because the client refuses outdoors-only spells itself.

## Install

Clone it into your AzerothCore `modules` folder **as `mod-forever-shaman`**, without the repo's
`wow-` prefix. AzerothCore finds the module's entry point from the folder name.

```bash
cd <azerothcore>/modules
git clone https://github.com/buildthehomelab/wow-mod-forever-shaman.git mod-forever-shaman
```

Rebuild the server and copy `conf/mod_forever_shaman.conf.dist` to your config folder as
`mod_forever_shaman.conf`. No SQL.

## Settings

| Setting | Default | What it does |
|---------|---------|--------------|
| `ForeverShaman.GhostWolfSpeed.Enable` | `1` | Ghost Wolf follows mod-mount-scaling. With `0`, it's the stock 40%. |
| `ForeverShaman.GhostWolfSpeed.OutOfCombatOnly` | `1` | Only out of combat; in combat Ghost Wolf is the stock 40%. With `0`, the mount speed applies in combat too. |
| `ForeverShaman.GhostWolfGathering.Enable` | `1` | Mining works in Ghost Wolf (needs the client patch). |
| `ForeverShaman.GhostWolfIndoors.Enable` | `1` | Ghost Wolf works indoors (casting it there needs the client patch). |

## Optional client patch

`tools/patch-forever-shaman-dbc.sh` adds Ghost Wolf to Mining (all 6 ranks), creature mining and
Engineering salvage, and takes "outdoors only" off Ghost Wolf, in the client's Spell.dbc:

```bash
tools/patch-forever-shaman-dbc.sh <Spell.dbc> DBFilesClient
```

Only the newest client patch's copy of Spell.dbc is used, so run it on the Spell.dbc your current
patch already ships (with the other modules' changes) and put the result back in that patch.
Players who get the new patch should delete their `Cache` folder.

## License

MIT
