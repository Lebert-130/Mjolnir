# Mjolnir

A map editor that I'm making for GoldSrc. Originally it was going to be only for my game engine, KosmicTec, but since I am only one person working on my game engine,
I think it's going to take a pretty long time to finish everything, so I decided to also make a GoldSrc version of my map editor, as a way to open-source one of my tools early.

I am open-sourcing this tool in hopes that some humble programmer, probably from the GoldSrc community, could help me out and finish this tool. Not only it benefits me,
but I think it also benefits the GoldSrc community, because I know there's some people who find Hammer to be much easier to use than Trenchbroom, and there is a lack
of open-source tools that are similar to Hammer, so that's my other reason why I am open-sourcing this tool.

At this moment, it's at its prototype stages, or at alpha version, so it is very unfinished.

## Some building information

At the moment, you need Visual C++ 6.0, yes I know, it's too old, it's because my game engine will support pretty old versions of Windows, starting from Windows 98, so I made
everything use Visual C++ 6.0. But that's alright, I have no problem modernizing this tool just for the others who want to use this tool. I have plans to bring this tool to much more modern versions of Visual Studio. Once that's done, I will just remove the VC++ 6.0 project, since I am pretty sure once it gets modernized, it's going to use much more
modern standards of C++, and VC++ 6.0 doesn't support anything above C++98.

The wxWidgets version that this tool uses is wxWidgets 2.8.6, I have included the libraries inside because I know that compiling this widget toolkit can be a bit difficult, all you have to do is get the headers from somewhere. I might upgrade wxWidgets once I bring the tool to much more modern versions of Visual Studio.