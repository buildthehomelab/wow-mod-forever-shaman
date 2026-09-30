# Forever Shaman

An [AzerothCore](https://www.azerothcore.org/) (WotLK 3.3.5a) module for shaman changes on a
3.3.5 server. For now that's Ghost Wolf keeping up with level-scaled mounts. No client patch.

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

Ghost Wolf is still outdoors only and can still be cast in combat, as in stock. Mount-only
bonuses (Riding Crop, Mithril Spurs, Carrot on a Stick) don't apply to it, and other run speed
effects don't stack with it: the core takes the highest one. Improved Ghost Wolf and the Glyph
of Ghost Wolf work as before.

Without mod-mount-scaling (or with `MountScaling.Enable = 0`) nothing changes.
[mod-forever-druid](https://github.com/buildthehomelab/wow-mod-forever-druid) does the same for
Travel Form and the flight forms.

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

## License

MIT
