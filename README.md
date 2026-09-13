Crossplatform TUI Podcast show manager/downloader with full mouse support
(not a player, use [musikcube](https://github.com/clangen/musikcube/blob/master/README.md)!)

```
╭───────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────┬───╮
│Add podcast...     ││Status: │New│Queue│Skipped│Done│All│                       │ X │
│All                │╰────────┴───┴─────┴───────┴────┴───┴───────────────────────┴───╯
│Les Grosses Têtes  │╭───────────────────────────────────────┬────────────┬──────────╮
│À bientôt de te rev││Title                                  │Date        │Duration ↑│
│FloodCast          │├───────────────────────────────────────┼────────────┼──────────┤
│2 Heures De Perdues││L'intégrale : "La dernière" du 06 septe│2026-09-06 2│1:46:09   █
│Riviera Detente    ││L'intégrale : "La dernière" du 13 septe│2026-09-13 2│1:45:45   █
│Un Bon Moment avec ││Le prix du gros mytho de la semaine est│2026-09-13 1│7:32mn    █
│Laisse-moi kiffer  ││Le prix du meilleur début de campagne e│2026-09-06 1│6:26mn    █
│Les Gens Qui Douten││Police partout, racisme aussi - La gros│2026-09-13 1│6:19mn    █
│Lumières dans la nu││Attal, l’opération 1000 bistrots - La c│2026-09-13 2│6:12mn    █
│Culture 2000       ││Arrêtons Mélenchon ! - La grosse semain│2026-09-06 2│5:52mn    █
│Émotions           ││Le charisme (d’huître) de Raphaël Gluck│2026-09-06 2│5:51mn    █
│Le Cosy Corner     ││Le grand pardon - La chronique de Thoma│2026-09-06 1│5:46mn    █
│Pardon GPT         ││L’autre 11 septembre - La chronique de │2026-09-13 1│5:32mn    █
│La dernière        ││Celles et ceux qui ont besoin d’un héro│2026-09-06 2│5:26mn    │
│Gamberge           ││Le RN et le voile - La chronique de Dja│2026-09-06 2│5:23mn    │
│Small Talk - Konbin││Celles et ceux qui ne sont pas des anim│2026-09-13 1│5:15mn    │
│Totemic            ││Fumer la cigarette électronique c’est f│2026-09-13 2│4:58mn    │
│Si c'est vrai c'est│╰─▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄────────────────────────────────────╯
│Hollywood French Lo│╭───────────┬────────┬──────────┬──────────┬────────────────────╮
│4 quart d'heures   ││Refresh (r)│Edit (e)│Delete (d)│Update (u)│                    │
│Laurent Baffie     │╰───────────┴────────┴──────────┴──────────┴────────────────────╯
│                   │╭───────────────────────────────────────────────────────────────╮
│                   ││Ready                                                          │
╰─▄▄▄▄▄▄▄▄▄─────────╯╰───────────────────────────────────────────────────────────────╯
```
```
╭───────────────────╮╭────────┬───┬─────┬───────┬────┬───┬───────────────────────┬───╮
│Add podcast...     ││Status: │New│Queue│Skipped│Done│All│                       │ X │
│All                │╰────────┴───┴─────┴───────┴────┴───┴───────────────────────┴───╯
│Les Gro╭────────────────────┬───────────────────────────────────────────────╮───────╮
│À bient│URL                 │https://podcasts.nova.fr/radio-nova-la-derniere│ation  │
│FloodCa│Title               │La dernière                                    │───────┤
│2 Heure│Target              │E:\podcasts\der                                │2mn    █
│Riviera│Pattern             │{date}-novader-{title}                         │7mn    │
│Un Bon │Description         │<p>Rendez-vous le dimanche en direct de la Radi│7mn    │
│Laisse-│Preview             │L'intégrale : "La dernière" du 13 septembre 202│9mn    │
│Les Gen│                    │Attal, l’opération 1000 bistrots - La chronique│3mn    │
│Lumière│                    │Fumer la cigarette électronique c’est fumer - L│4mn    │
│Culture│                    │Police partout, racisme aussi - La grosse semai│6mn    │
│Émotion│                    │Celles et ceux qui ne sont pas des animaux - La│0mn    │
│Le Cosy│                    │L’autre 11 septembre - La chronique de Mathilde│5mn    │
│Pardon │                    │Les nazis, c’était mieux avant ! - La chronique│7mn    │
│La dern│                    │Hollande, ô Désespoir ! - La chronique de Flore│8mn    │
│Gamberg│                    │Le prix du gros mytho de la semaine est attribu│9mn    │
│Small T│                    │L'intégrale : "La dernière" du 06 septembre 202│6mn    │
│Totemic│                    │La perceuse pour femmes - La chronique de Mamar│2mn    │
│Si c'es│                    │Celles et ceux qui ont besoin d’un héros - La c│───────╯
│Hollywo│Save podcast (s)    │Cancel (Esc)                                   │───────╮
│4 quart╰─▄▄▄▄▄▄▄▄▄▄▄▄▄──────────────────────────────────────────────────────╯       │
│Laurent Baffie     │╰───────────┴────────┴──────────┴───────────────────────────────╯
│                   │╭───────────────────────────────────────────────────────────────╮
│                   ││Ready                                                          │
╰─▄▄▄▄▄▄▄▄▄─────────╯╰───────────────────────────────────────────────────────────────╯
```

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

