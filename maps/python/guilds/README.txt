Python Guilds

Quick outline

Add a guild (mapmakers):  To add a guild copy the guild maps from the templates/guild folder, change the exit to the world on mainfloor, find and replace GUILD_TEMPLATE in the maps with the name of your guild and add the guild name to the templates/guild/GuildList file (one guild name per line).

Buy a guild (players): Find an inactive guild and say buy (Guardian will tell you if the guild is inactive or not).  You will need three players and 5000 imperials to purchase a guild.

Manage a guild (players): Use the Guild Oracle in the guild HQ.  Only GMs can enter here.  You can remove members, change their status (good, probation or suspended), promote ore demote members or get info on members or a member.

Manage a guild (DMs): use the Guild Oracle to change the status of the guild (probation, suspended, good, inactive).  DMs can also add members to the guild or use the GM guild functions but SHOULD NOT in most cases.

Join a guild(players):  Player enters Hall of joining and sits in chair of peril.  a GM must pull the 'lock' lever to secure the player, then the 'load' lever to have them join the guild.  

Pay Dues(players): Paying Dues and doing guild quests are the way to get noticed and promoted in your guild.  Differnet guilds have different requirements but all gain status by having members generating dues and quest points.

QuestPoints(mapmakers): put a call to the appropriate guildquest script (currently only guild_questpoints_apply.py but a 'say' script coming soon...) to give guild quest points in your maps.  In the plugin options you will have to specify the number of points to be given. They should be appropriate to the task.  Quests giving points should not be easily repeatable and should try to avoid things like players camping or knowledge based access (don't use hidden or password activated only).


########################
Brief history of development

Update 07-02-12

Also being documented at:
http://wiki.metalforge.net/doku.php/dev_todo:python_guilds

Merged in map updates and changes, which include:

==== What Works ====

  * secondfloor, drop 20 amulets of Lifesaving for a portal to the Mazes of Menace
    * Two entrances appear for some reason though
  * mainfloor, drop 1000 platinum for a silver to gold converter
  * mainfloor, drop 1000 platinum for a gold to platinum converter
  * mainfloor, drop 1000 platinum coins for a platinum to gold converter
  * mainfloor, drop 5000 Platinum for a portal to Scorn
  * mainfloor, drop 1000 diamonds for a portal to Brest
  * mainfloor, drop 5000 pearls for a portal to Santo Dominion
  * mainfloor, drop 2 dragon mail for a portal to Navar
  * mainfloor, drop 5 tissue paper for a portal to the Pupland Terminal (works, but is asking for the wrong item/material)
  * mainfloor, drop a shootingstar for a portal to Nurnberg
  * mainlfoor, drop an Unholy Bow for a portal to Ancient Pupland
  * mainfloor, Kennel token
  * guild_HQ, 'sayGM Message board
  * basement, drop 25 potions of fire resistance for an altar of Ixalovh
  * basement, drop 25 potions of cold resistance for an altar of Gorokh
  * basement, drop 50 pixie wings for an altar of Gnarg
  * basement, drop 3 Bonecrushers for an altar of Mostrai
  * basement, drop 15 healing potions for an altar of Gaea
  * basement, drop 14 Elven Bows for an altar of Lythander
  * basement, drop 6 mjoellnirs for an altar of Sorig
  * basement, drop 5 Demonbanes for an altar to Valriel
  * basement, drop 5 Firebrands for an altar of Ruggilli
  * basement, drop 5 Frostbrands for an altar to Devourers
  * basement, drop a Firestar named Fearless for the Big Chest
  * Other areas have not been tested

==== Incomplete ====

"Broken Altars" - it appears that it has been undecided on what exactly should be used or dropped to gain access to the following:

  * mainfloor, drop x for a mailbox
    * This works, just need to determine what "x" is
  * mainfloor, drop x for basement stairs
    * This works, just need to determine what "x" is
  * mainfloor, drop x for a forge
    * Requires the "connection" to be re-matched between the altar <-> creator and then between the check inventory <-> gate, the keystring is already matched correctly
    * Forge room works now, just need to determine what "x" is
  * mainfloor, drop x for workbench
    * Requires the "connection" to be re-matched between the altar <-> creator and then between the check inventory <-> gate, the keystring has to be updated as well between the creator <-> check inventory
    * Workbench room works now, just need to determine what "x" is
  * mainfloor, drop x for a message board
  * mainfloor, drop x for stove
    * Requires the "connection" to be re-matched between the altar <-> creator, inventory check between inventory <-> gate worked
    * after purchasing the altar, a woodfloor tile drops in it's place and surrounded by grass; this is still a problem
    * after dropping the token to gain access to the stove area, a woodfloor now appears which is intended (fixed)
  * mainfloor, Toolshed Token (found in Guild_HQ)
    * Note: the first gate opens for everyone, it's the second gate that requires the Toolshed token to be turned in for it to open
    * mis-matched slaying field on the altar (slaying Toolshed_token) and the name of the Token (Toolshed Token)- now fixed
      * Fixed the problem where other nearby pay altars would disappear when the toolshed was "purchased"
  * mainfloor, Garden Token (found in Guild_HQ)
    * mis-matched slaying field on the altar (slaying garden_token) and the name of the Token (Garden Token)
  * mainfloor, drop 5 tissue paper for a portal to the Pupland Terminal - requires bolt_silk instead of tissue paper
  * secondfloor, drop 20 amulets of Lifesaving for a portal to ?
    * This drop spot disappears and is replaced with a second or additional portal to the Mazes of Menace (when 20 amulets of Lifesaving are dropped on it)
  * secondfloor, (x 15 y 7) drop x for Alchemy room
    * Alchemy room works now, just need to determine what "x" is
  * secondfloor, (x 11 y 1) drop x for Glowing Crystal Room
    * This was updated to allow access to the Glowing Crystal room (was listed as Alchemy room)
    * Also, one of the tokens from the Guild_HQ was updated and is required as a turn in to gain access to the Glowing Crystal Room
    * Glowing Crystal Room works like many of the skill areas, pay a price for "day pass" to access the crystal; also have multiple gates in place to protect players and items from the mana explosion
  * secondfloor, drop x for Jewelers room
    * Jewelers room works now, just need to determine what "x" is
  * secondfloor, drop x for Thaumaturgy room
    * Thaumaturgy room works now, just need to determine what "x" is
  * basement, drop 10 gold coins (for what?)
  * basement, dropping the Firestar named fearless allows access to BigChest, but I suspect that the drop location of the chest is not as intended because the player is in the way
  * bigchest, once you enter the chest the exit back to the basement is broken ("closed")
  * Other areas untested

In the far upper right of the map, things such as the altars, stairs, message board, et al. reside on teleporters which then "transport" items to the map as they are obtained (drop x gold, return with a spectre, etc.) - those map tiles are not unique.  So, when the map is reset (which is normal), already purchased items reappear in that area.  From my testing, this does not cause any problems at the moment.  But this should be corrected/fixed.

Major chore: placing all the no_magic tiles under the regular floor tiles so they do not show up with the show invisible spell

Update 07-02-12

Fixed typos, it's now possible to buy guilds.

Ryo

Update 05-12-04

Uploaded guild package to CVS.
Changes - the cauldrons and the charging room I spent a while agonizing over. They were natural guild enhancements but much too much value for any reasonable expense to buy them. Then I thought that they should be pay access but at a greatly reduced rate. SO when you buy a forge or whatever for your guild it is available on a perplayer daily rate of 50pl (compared to 200pl for access in Scorn). This is I think a good comprimise. The charging room should work the same way but be more expensive (?). Changing this in the maps took me some time so I didn't get a chance to test all the connects yet however. I still wanted to get the maps and scripts in to CVS so they could be versioned.
The other change was to the storage house. I basically removed the separate storage house and added member 'lounges' for all the different guild member ranks above the first (Initiate) - Ryo's building system will get more use this way and players will have another reward for advancement. - a custom lounge. There are no links to the lounges from the main guild maps yet.

Update 04-11-30

As soon as I write the DM command script and double check my names I will commit this to CVS. I'm adding the scripts under /python naturally and will commit a folder called templates/guild - the guilds will still not be in the game unless the admin puts them in, but it will be accessable for testing and to DMs to play with. Like I said lots still to do with the maps, especially comingt up with quest items for buying things like the new workshops and stuff.

One of the things I would like some input on would be proposals for additional fields for either the guildhouses or guild datafiles to play with.

Currently the Guildhouse is:

NAME | FOUNDED_DATE | POINTS | STATUS

of which only points is really used for game play (the rest are administrative) . Points is directly updated when guild members pay dues - this was to pull money out of the economy, but there is no reason we can't have more than one measure of a guild performance, perhaps have dues relate to 'Dues' and use points for some other reward (DM awards, some other measure?) Status is set by the DM (probation, suspended or inactive) or when a guild is founded inactive -> active

Guilds have the format:

NAME | JOIN_DATE |RANK | STATUS | DUES | DEMERITS

Which has more measures (Demerits and Rank namely) but which naturally could have more to drive more functions. Basically I see it working that Rank is gained as members pay dues and otherwise do stuff for the guild (the guild would determine how much and what) and that demerits are applied a worked off as a precondition for adbvancement or even palyer status. Status is set by higher ranked players and players can be put on probation or even suspended from the guild for accumulating demerits or other infractions.

So before this is committed I wouldn't mind a few comments - just don't suggest stuff that is going to require a complete rewrite or anything. Adding a new field is easier now then later when a conversion of existing records would be required.

Update 04-11-29

Fixed nasty bug where find_player on square functions crashed server.  Fixed up the hall of joining so it works nice (pull Lock to secure member, pull Load to join em up.)  The maps still need some work and there are probably more little functions here and there (especially DM toys) but the *basic* code stuff is all done IMHO. You can add guilds, buy guilds, add new members, pay dues administrate... also the plain text format allows for ease of editing.
I'm thinking about adding a CFFindPlayerOnSquare() and CFGetDate(), CFGetTime() functions to the python plugin.  Useful for  mail system, Guild foundings and other python scripts.

Update 04-11-28

Added in scripts for guild joining (the chair), and removing player who quit form the guilds they belong to. Renamed guildmember_say to guildoracle.py and fixed some bugs here and there. Made guild board script that displays the points of the guilds in decending order.

*04-11-28

more to clean up still, have to write bit that checks if player is already in a guild as well as hook up joingroom chair to a script.

04-11-27

Modified the guild entry script so that you can buy a guild on site instead of using a guildhousesinc type building. This should make adding a guild as simple as putting the maps somewhere, making up a name (probably use a script to change the maps...) and adding it to the GuildList file. It also gets rid of keys entirely and ensures that you can't get a guild unless the Python functions are working to run it. Also added in methods for DMs to put a guild on probation or suspension if desired. The current price for one of these new guilds is 5000 imperials (or 1,000,000 platinum) - split among three founders, I think that's fair. This is negotiable (anyone have any ideas on this?) and easily modified naturally.

Update 04-11-22

So up late last night making changes to the guild map, basically moving the sleeping area to another floor(unfinished) and making the main room bigger. Put a dues collector in the corner below the GM entry to the HQ. I also got the GuildList thing working so it reads in the list of guild from the map set so that a map maker can add a guild pretty easily without having to touch the server stuff.
I added in a status check to the guild entry so that members on probation get a warning and members who are suspended aren't allowed in.
Still need to make a guild buying part and work over the guild maps some more.
By the end of this weekend I hope I'll have a barebones working package for the code (and map templates) which I could commit to CVS which will make it much easier for us to work with/on. If anyone is working on the guild maps I would like to touch base with them before so that at least the files are named. It's easier to add and edit files than delete or move/rename them in CVS.


Update 04-11-22

It's been quiet around here so I've been puttering away at things a bit. Wrote scripts for collecting dues and for managing the guild houses. I'm going to propose that a new folder be put into the maps dir called templates where reusable maps can be stored (personal houses, castles, temples...) and one of the subfolders be called 'guilds' which will hold the master guild template and a file (GuildList) which will have a list of all the guilds deployed in the server mapset. The guildhouses scripts read in this file and adds any new guilds if they are added to the file but not in the guild system datafile (easy way to add new guilds to the system). This guildhouse list also holds points for the guilds, a founding date and a status. A few more things to do hopefully before this weekend to manage guild membership - like player quit event and a way to check if a player belongs to a guild when joining (I believe you should only belong to one guild at a time - anyone want to debate?). I am also making some of the guild maps a bit bigger to accommodate more hanging out space (hey CF space is free ...) and to give a place for things like the dues collector and meeting rooms for members of different ranks (like The Journeymans Lounge...).

Update 04-11-21

Ok, I've done a bit more work and implemented guild ranks, status and various other gizmos. I changed the list function in CFDataFile to use dictionary keys instead of iteration because python 2.1 does not have dictionary sequence method and as I've said I want to keep crossfire 2.1 compatible (at least till Debian Sarge is mainstream anyway). I've also started on python scripts for buying a guild, a guildhouses info board which will show scores and stuff and a tiny bit of the Dues stuff. I think I would like to place some 'tokens' for some of the bigger guild enhancements (like the kennels, the garden, additional levels...) on the HQ map itself and make them accessable (a scripted lever perhaps) only when a certain number of dues have been paid by the members (many many imperials perhaps?) Naturally other things should be quest items.
I noticed that the guild master message board was changed to a second guild controll station and the guild message board in the HQ was changed to a GM message board - perhaps this was a misunderstanding? The GM board is for GM messages and the other is for the regular members. Also the cauldrons on the second floor don't seem to have a way to buy entry anymore. I don't know if the charging room should be key driven because keys are easily lended out and can't be recovered unless the lendee want's to return them, maybe we can use Rank for this (only guildsmen or above get access) or use a permemant maker instead of a key. I also want to do away with the keys altogether and make the code initiate the three guild masters when the guild I bought.
