Crossplatform TUI Podcast show manager/downloader with full mouse support
(not a player, use [musikcube](https://github.com/clangen/musikcube/blob/master/README.md)!)

Watch [demo here](https://asciinema.org/a/pnfvhCQvSaICCxSA)

```
╭─────────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────────┬───╮
│Add podcast...       █│Status: │New│Queue│Skipped│Done│All│                           │ X │
│All                  █╰────────┴───┴─────┴───────┴────┴───┴───────────────────────────┴───╯
│Les Grosses Têtes    █╭───────────────────────────────────────────┬────────────┬──────────╮
│À bientôt de te revoi█│Title                                      │Date        │Duration  │
│FloodCast            █├───────────────────────────────────────────┼────────────┼──────────┤
│2 Heures De Perdues  █│Kayane, 24 ans, profession : joueuse de jeu│2016-06-08 2│0s        █
│Riviera Detente      █│Comment l’assassinat de JFK nous a fait ent│2016-06-08 2│0s        │
│Un Bon Moment avec Ky█│Europe 1 social club – 07/06/16            │2016-06-08 2│0s        │
│Laisse-moi kiffer    █│Europe 1 social club – 06/06/16            │2016-06-07 2│0s        │
│Les Gens Qui Doutent █│Europe 1 social club – 03/06/16            │2016-06-04 2│0s        │
│Lumières dans la nuit█│Europe 1 social club – 02/06/16            │2016-06-03 2│0s        │
│Culture 2000         █│"Aujourd'hui, chaque joueur de football est│2016-06-02 2│0s        │
│Émotions             █│Europe 1 social club – 01/06/16            │2016-06-02 2│0s        │
│Le Cosy Corner       █│"Ils sont partout" : Yvan Attal veut "démon│2016-06-01 2│0s        │
│Pardon GPT           █│Europe 1 social club – 31/05/16            │2016-06-01 2│0s        │
│La dernière          █│Jean-Louis Servan-Schreiber est devenu "qua│2016-05-27 2│0s        │
│Gamberge             █╰██████████████████████████████████████─────┴────────────┴──────────╯
│Small Talk - Konbini █╭───────────────────────────────────────────────────────────────────╮
│Totemic              █│                                                                   │
│Si c'est vrai c'est t█╰───────────────────────────────────────────────────────────────────╯
│Hollywood French Love│╭───────────────────────────────────────────────────────────────────╮
│4 quart d'heures     ││Ready                                                              │
╰███████████──────────╯╰───────────────────────────────────────────────────────────────────╯
```
```
╭─────────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────────┬───╮
│Add podcast...       █│Status: │New│Queue│Skipped│Done│All│                           │ X │
│All       ╭────────────────────────────────────────────────────────────────┬───╮──────┴───╯
│Les Grosse│Edit podcast                                                    │ X │──────────╮
│À bientôt ├────────────────────┬───────────────────────────────────────────┴───┤Duration  │
│FloodCast │URL                 │https://podcasts.nova.fr/radio-nova-la-derniere█──────────┤
│2 Heures D│Title               │La dernière                                    █4:22mn    █
│Riviera De│Target              │E:\podcasts\der                                █4:57mn    │
│Un Bon Mom│Pattern             │{date}-novader-{title}                         █3:37mn    │
│Laisse-moi│Description         │<p>“Comme on a compris que la liberté d’express█5:59mn    │
│Les Gens Q│Preview             │                                               █1:47:25   │
│Lumières d│                    │                                               █6:13mn    │
│Culture 20│                    │                                               █4:14mn    │
│Émotions  │                    │                                               █3:36mn    │
│Le Cosy Co│                    │                                               █4:10mn    │
│Pardon GPT│                    │                                               █5:45mn    │
│La dernièr│                    │                                               █1:46:51   │
│Gamberge  │                    │                                               │──────────╯
│Small Talk│                    │                                               │──────────╮
│Totemic   ├────────────────────┴────────────────────────────────────────┬──────┤          │
│Si c'est v│OK                                                           │Cancel│──────────╯
│Hollywood ╰─────────────────────────────────────────────────────────────┴──────╯──────────╮
│4 quart d'heures     ││Ready                                                              │
╰███████████──────────╯╰───────────────────────────────────────────────────────────────────╯
```
```
╭─────────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────────┬───╮
│Add podcast...       █│Status: │New│Queue│Skipped│Done│All│                           │ X │
│All       ╭────────────────────────────────────────────────────────────────┬───╮──────┴───╯
│Les Grosse│Select target folder                                            │ X │──────────╮
│À bientôt ├──────────────────────────────────────────────┬──────────┬──────┴───┤Duration  │
│FloodCast │Name                                          │Type      │Ext       │──────────┤
│2 Heures D├──────────────────────────────────────────────┼──────────┼──────────┤4:22mn    █
│Riviera De│[D] ..                                        │Directory │          █4:57mn    │
│Un Bon Mom│[D] .git                                      │Directory │          █3:37mn    │
│Laisse-moi│[D] .vscode                                   │Directory │          █5:59mn    │
│Les Gens Q│[D] build-windows                             │Directory │          █1:47:25   │
│Lumières d│[D] build-windows-debug                       │Directory │          █6:13mn    │
│Culture 20│[D] build-wsl                                 │Directory │          █4:14mn    │
│Émotions  │[D] build-wsl-debug                           │Directory │          █3:36mn    │
│Le Cosy Co│[D] cmake                                     │Directory │          █4:10mn    │
│Pardon GPT│[D] src                                       │Directory │          █5:45mn    │
│La dernièr│[D] tuimm                                     │Directory │          │1:46:51   │
│Gamberge  │[F] .gitattributes                            │File      │          │──────────╯
│Small Talk│[F] .gitignore                                │File      │          │──────────╮
│Totemic   ├──────────────────────────────────────────────┴──────────┴───┬──────┤          │
│Si c'est v│OK                                                           │Cancel│──────────╯
│Hollywood ╰─────────────────────────────────────────────────────────────┴──────╯──────────╮
│4 quart d'heures     ││Ready                                                              │
╰███████████──────────╯╰───────────────────────────────────────────────────────────────────╯
```
```
╭─────────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────────┬───╮
│Add podcast...       █│Status: │New│Queue│Skipped│Done│All│                           │ X │
│A╭──────────────────────────────────────────────────────────────────────────────────┬───╮─╯
│L│#141 - Cosy Lundi                                                                 │ X │─╮
│À├──────────────────────────────────────────────────────────────────────────────────┴───┤ │
│F│                                                                                      █─┤
│2│[00:00:00] plancha burger et poker                                                    █ █
│R│[00:24:02] la farandole des cons, et redevenir humain                                 █ │
│U│[00:51:55] Manor Lords                                                                █ │
│L│[01:18:27] Comics et marchands de journaux                                            █ │
│L│[01:51:42] Fallout (série)                                                            █ │
│L│[02:08:42] Eiyûden Chronicle : Hundred Heroes                                         █ │
│C│[02:30:20] Remerciements                                                              █ │
│É│                                                                                      █ │
│L│La page Patreon du Cosy Corner : https://www.patreon.com/lecosycorner                 █ │
│P│                                                                                      █ │
│L│-- Playlist --                                                                        █ │
│G│                                                                                      │─╯
│S│- My Bloody Valentine - Only Shallow                                                  │──
│T│- Placebo - Burger Queen                                                              │(-
│S│- Pavement - Cut Your Hair                                                            │──
│H╰──────────────────────────────────────────────────────────────────────────────────────╯─╮
│4 quart d'heures     ││Ready                                                              │
╰███████████──────────╯╰───────────────────────────────────────────────────────────────────╯
```

Within VSCode console (WSL/Ubuntu)

![https://github.com/maudoin/casttui/blob/main/src/resources/screenshot.png?raw=true](https://github.com/maudoin/casttui/blob/main/src/resources/screenshot.png?raw=true)

Windows
==

- Update `MSYS2_PREFIX` in `cmake\toolchain-clang-win.cmake` to setup your mingw path if it's not in `PATH` and you don't run from Msys2 console (If you don't have any Msys2 UCRT, look here https://www.msys2.org/)
- Install Clang/OpenSSL with
```
pacman -Syu
pacman -S mingw-w64-ucrt-x86_64-clang
pacman -S mingw-w64-ucrt-x86_64-openssl
```
- Optional: install Debugger with
```
pacman -S mingw-w64-ucrt-x86_64-gdb
```

Note: above setup is using UCRT/Clang combination but GCC should be actually be used for Msys2 UCRT and Msys2 W64 version would be better for clang

WSL/Ubuntu
==
- You may install clang/cmake with
```
apt install clang
apt install cmake
```

Dependencies
==

| Dependency | Version | Download |
|-----------|---------|----------|
| [fmt](https://github.com/fmtlib/fmt) | 9.1.0 | https://github.com/fmtlib/fmt/archive/refs/tags/9.1.0.zip |
| [rapidxml](https://github.com/Fe-Bell/RapidXML) | 1.17 | https://github.com/Fe-Bell/RapidXML/archive/refs/tags/v117.zip |
| [sqlite3](https://www.sqlite.org) | 3.42.0 | https://www.sqlite.org/2023/sqlite-amalgamation-3420000.zip |
| [libhv](https://github.com/ithewei/libhv) | 1.3.0 | https://github.com/ithewei/libhv/archive/refs/tags/v1.3.0.zip |
| [pdcurses](https://github.com/wmcbrine/PDCurses) | 3.9 | https://github.com/wmcbrine/PDCurses/archive/refs/tags/3.9.zip |
| [httplib](https://github.com/yhirose/cpp-httplib) | 0.54.1 | https://raw.githubusercontent.com/yhirose/cpp-httplib/refs/tags/v0.54.1/httplib.h |
| [openssl](https://www.openssl.org) | latest | use distribution manager |

