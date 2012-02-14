=====================================
droceRoG - Go Game Record/Kifu Viewer
=====================================

Version: 0.1
Projected is hosted on https://github.com/chermes/droceRoG by GitHub

Contents
--------
1. Overview
2. Install the Program
3. Usage
4. Development
    4.1 Install PocketBook SDK
    4.2 Get droceRoG sources from GitHub
    4.3 Compile droceRoG
    4.4 Develop droceRoG
5. Authors/Developers
6. License
7. References

1. Overview
-----------
droceRoG is a viewer for SGF (Smart Go/Game Format) files on your PocketBook
Pro ebook reader [1]. The project name "droceRoG" is an artificial word,
spelled backwards "GoRecord".

SGF files [2] contain Go games, possibly with comments and board labels. The
game Go [3] is an ancient strategic board game originating in China about 3000
to 4000 years ago and still quite popular especially in Asian countries. The
rules are quite simple and easy to learn, but a deep understanding of the
strategy requires many years of practice. All in all, it's just fun to play.

The idea behind the development of droceRoG is to put your ebook reader beside
your Go board, and play, understand, and follow the documented game
interactively by placing stones on the board. Reviews of recently played games
by amateur players can be found on the Go Teaching Ladder website [4], but also
commented professional games can be found by your favorite internet search
engine. Just put the SGF files on your PocketBook and droceRoG will find them
automatically.

Features:
* Show the Go board independent of board size.
* Display markers in annotated review games
* Fast navigation (single move, go to next variation/comment, go-to-move
* dialog, switch between variations)
* Display comments (either under board or full screen)
* Show variation graph for overview and fast access

Currently, droceRoG is supported for the PocketBook Pro 6" and 9" series, but
still in development. If you have any suggestions, ideas or find bugs, please
report them in the GitHub Issues section [5]. A PocketBook executable will be
released when a stable version 1.0 is reached, major bugs have been fixed, and
the main functionality is implemented (hopefully, soon) but unstable releases
0.X are published before.

2. Install the Program
----------------------
There are official compiled releases on GitHub [6] (droceRoG_*.zip), but you
can also compile the program for yourself (see section "Development"). In both
cases, the installation procedure is the same:

For installing droceRoG, you need the following files:
* drocerog.app
* fonts/drocerog.ttf
* fonts/DejaVuSerif.ttf

In the following, I will describe how to put the program (either downloaded or
self-compiled) on your PocketBook. 

* [required] Connect the ebook with your computer via USB cable. Let's assume
  it has been mounted on /media/PocketBook (Linux) or E:\PocketBook (Windows).
  Both mounting points are labeled "/PB_Mount/" in the following description.

* [required] Put the executable "droceRoG.app" to /PB_Mount/system/bin/

* [required] Put the font files "drocerog.ttf" and "DejaVuSerif.ttf" to
  /PB_Mount/system/fonts/

* [required] For directly open a SGF with droceRoG in the PocketBook explorer
  by clicking on it, edit the file "/PB_Mount/system/extensions.cfg". If the
  file doesn't exist, create it. Add the following line:
  sgf:@SGF:1:drocerog.app:ICON_APP

* [optional] If you want to see droceRoG in your applications directory, create
  the file /PB_Mount/applications/drocerog.app with the following content:
  #!/bin/sh
  exec /ebrmain/bin/drocerog.app

3. Usage
--------
droceRoG has a build-in help for explaining the keys. Just click the context
menu button (book sign) and select "Show help". 

Right-hand keys:
* Home - Exits the program and returns to PocketBook intro screen.
* Menu - Opens context menu (file selection, go to move, etc.)
* Forward / Backward - One move forward / backward
* OK - Displays a comment on the full screen instead under the board

Navigation keys:
* Left / Right - Move to previous / next variation or comment
* Up / Down - Switch between variations
* Return - Exit program

4. Development
--------------
This section is for all people who want to mess up the source files, compile,
or run droceRoG on their local computer with linux installed (in fact, I use
Ubuntu). First, you will need the PB SDK and then download the droceRoG sources
from GitHub. I assume that you know the basics of software development, have
root permissions for the SDK requirements and know how to use a shell.

4.1 Install PocketBook SDK
--------------------------
PocketBook provides a simple SDK at SourceForge which can be installed without
root permissions. Go to [7] and download the file "sdkrelease_1_1a.tar.gz"
(maybe there is a newer version available).

Follow the readme.txt instructions:

> To start working with SDK you should do the following: 
> 1) Unpack the archive into ~/projects:
>    mkdir ~/projects && tar zxvf sdkrelease_1_1a.tar.gz -C ~/projects/ 
> 
> 2) install Packages:
>    sudo apt-get install build-essential cmake autoconf qtcreator
>    libgtk2.0-dev libbz2-dev libcurl4-openssl-dev libgif-dev libjpeg62-dev
>    [please note: libjpeg62-dev is not mentioned in original readme.txt]
> 
> 3) Run the following commands:
>    sudo -s
>    cd /usr/include && ln -sf freetype2/freetype freetype

4.2 Get droceRoG sources from GitHub
------------------------------------
Move to your sources directory:
    cd ~/projects/sources/

Either manually download the sources from GitHub [8] or by using git:
    git clone git://github.com/chermes/droceRoG.git

Set up your local PB system ~/projects/system according to [2. Install the
Program] with one exception: Do not copy the executable "drocerog.app".

4.3 Compile droceRoG
--------------------
There are two compilation modes:

1) Compile for your PocketBook, creates "drocerog.app":
    sh makearm.sh

2) Compile for your local machine, creates "drocerog.exe":
    sh makepc.sh
   You're now able to test droceRoG without the ebook reader.

4.4 Develop droceRoG
--------------------
Help is highly appreciated. The project has a simple structure:

src/    Main source files.
sgf/    SGF file reading lib adopted from the GnuGo project [9].
fonts/  Instead of using fixed-size bitmaps, droceRoG uses own designed,
        scalable fonts. The main font file is "drocerog.sfd" and can be opened
        by FontForge [10]. After editing, the font can be exported to
        "drocerog.ttf".

At this point, an excuse: The content of the source files is not very
structured, rather just a collection of methods. But it seems to work... :-P

5. Authors/Developers
---------------------
* Christoph Hermes (hermes<AT>hausmilbe<DOT>net)

6. License
----------
GPL 3, see LICENSE.txt for details.

The droceRoG project uses the SGF I/O library from the GnuGo project, which is
also licensed under the GPL. Some of the files have been modified to own needs,
so an own version is shipped with the droceRoG source code.

7. References
-------------

[1]  http://www.pocketbook-int.com/us
     PocketBook ebook reader website, overview of different models
[2]  http://www.red-bean.com/sgf/
     SGF file format definition
[3]  https://en.wikipedia.org/wiki/Go_%28game%29
     Good overview about the Go game
[4]  http://gtl.xmp.net/
     The Go Teaching Ladder; many reviews by strong amateur players
[5]  https://github.com/chermes/droceRoG/issues
     Put issues, ideas and bug reports here.
[6]  https://github.com/chermes/droceRoG/downloads
     Download compiled droceRoG releases.
[7]  http://sourceforge.net/projects/pocketbook-free/files/PocketBook_Pro_SDK_Linux_1.1/
     PocketBook Pro SDK
[8]  https://github.com/chermes/droceRoG/zipball/master
     Download droceRoG sources as a zip ball
[9]  https://www.gnu.org/software/gnugo/
     GnuGo project
[10] http://fontforge.sourceforge.net/
     FontForge: create, edit, and export font files to TTF

= END OF FILE =
