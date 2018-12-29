# -*- coding: utf-8 -*-
# CFDialog.py - Dialog helper class
#
# Copyright (C) 2007 Yann Chachkoff
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#
# The author can be reached via e-mail at lauwenmark@gmail.com

# What is CFDialog?
# =================
#
# This is a small set of utility classes, to help you create complex dialogs.
# It is made for those who do not want to bother about complex programming,
# but just want to make a few dialogs that are better than the @match system
# used in the server.
# You will not normally use this directly, but will instead want to call 
# dialog/npc_dialog.py which will handle most common uses for dialogs.
#
# How to use CFDialog
# ===================
#
# First, create a script that imports the DialogRule and Dialog classes. Add
# the following line at the beginning of your script:
#
#   from CFDialog import DialogRule, Dialog
#
# Next, build the dialog by creating a sequence of several rules made up of
# keywords, answers, preconditions, and postconditions.  
#
# - Keywords are what the rule answers to.  For example, if you want a rule to
#   trigger when the player says "hi", then "hi" must appear in the keyword
#   list.  One or more keywords are specified in a string list in the form
#   ["keyword1", "keyword2" ...].  A "*" character is a special keyword that
#   means: "match everything", and is useful to create rules that provide
#   generic answers no matter what the player character says.
#
#   NOTE:  Like the @match system, CFDialog converts both keywords and the
#          things the player says to lowercase before checking for a match,
#          so it is never necessary to include multiple keywords that only
#          differ in case.
#
# - Answers are what the rule will respond, or say, to the player when it is
#   triggered.  This is what the NPC replies to the player. Answers are stored
#   in a list of one or more strings in the form ["Answer1", "Answer2" ...].
#   When there is more than one answer in that list, each time the rule is
#   triggered, a single random reply will be selected from the list.
#
#   NOTE:  Answers may contain line breaks.  To insert one, use "\n".
#
# - Preconditions are checks that must pass in order for a rule to be 
#   triggered. The checks that can be used are to be found in dialog/pre/*.py
#   Each file describes how to use the check in question.
#
# - Postconditions are changes that should be made to the player and/or the 
#   game world after the rule triggers. The effects that are available are to
#   be found in dialog/post/*.py Each file describes how to use the effect in
#   question.
#
# - Replies are what the player will be informed of possible replies.
#   Each should be an array in the form [word, text, type], with
#   'word' the actual word the player should say, 'text' the text the player
#   will actually say if she says the word, 'type' an optional integer
#   to specify if the text is a regular sentence (0), a reply (1) or a question
#   to ask (2).
#
#
# Once the rules are all defined, assemble them into a dialog.  Each dialog
# involves somebody who triggers it, somebody who answers, and also a unique
# name so it cannot be confused with other dialogs.  Typically, the "one who
# triggers" will be the player, and the "one who answers" is an NPC the player
# was taking to. You are free to choose whatever you want for the dialog name,
# as long as it contains no whitespace or special characters, and as long as
# it is not used by another dialog.  You can then add the rules you created to
# the dialog. Rules are parsed in a given order, so you must add the most
# generic answer last.
#
#   http://wiki.metalforge.net/doku.php/cfdialog?s=cfdialog#a_simple_example
#
# A more complex example
# ======================
#
# A ./misc/npc_dialog.py script has been written that uses CFDialog, but
# allows the dialog data to be written in JSON format.
# This also permits the inclusion of additional files to take in more rules 
# (this is mostly useful when you have a character who has some specific lines 
# of dialog but also some other lines that are shared with other characters 
# - the character can reference their specific lines of dialog directly and 
# include the general ones. 
#
# ../scorn/kar/gork.msg is an example that uses multiple keywords and multiple
# precondition values.  Whereas the above example has a linear and predicable
# conversation paths, note how a conversation with Gork can fork, merge, and
# loop back on itself.  The example also illustrates how CFDialog can allow
# dialogs to affect how other NPCs react to a player.  ../scorn/kar/mork.msg
# is a completely different dialog, but it is part of a quest that requires
# the player to interact with both NPCs in a specific way before the quest
# prize can be obtained.  With the old @match system, once the player knew
# the key words, he could short-circuit the conversation the map designer
# intended to occur.  CFDialog constrains the player to follow the proper
# conversation thread to qualify to receive the quest reward.
#
# Debugging
# =========
#
# When debugging, if changes are made to this file, the Crossfire Server must
# be restarted for it to register the changes.

import Crossfire
import string
import random
import sys
import os
import CFItemBroker

class DialogRule:
    def __init__(self, keywords, presemaphores, messages, postsemaphores, suggested_response = None):
        self.__keywords = keywords
        self.__presems = presemaphores
        self.__messages = messages
        self.__postsems = postsemaphores
        self.__suggestions = suggested_response
        self.__prefunction = None

    # The keyword is a string.  Multiple keywords may be defined in the string
    # by delimiting them with vertical bar (|) characters.  "*" is a special
    # keyword that matches anything.
    def getKeyword(self):
        return self.__keywords

    # Messages are stored in a list of strings.  One or more messages may be
    # defined in the list.  If more than one message is present, a random
    # string is returned.
    def getMessage(self):
        msg = self.__messages
        l = len(msg)
        r = random.randint(0, l - 1)
        return msg[r]

    # Return the preconditions of a rule.  They are a list of one or more lists
    # that specify a flag name to check, and one or more acceptable values it
    # may have in order to allow the rule to be triggered.
    def getPreconditions(self):
        return self.__presems

    # Return the postconditions for a rule.  They are a list of one or more
    # lists that specify a flag to be set in the player file and what value it
    # should be set to.
    def getPostconditions(self):
        return self.__postsems

    # Return the possible responses to this rule
    # This is when a message is sent.
    def getSuggests(self):
        return self.__suggestions

    # Return a possible pre function, that will be called to ensure the rule matches.
    def getPreFunction(self):
        return self.__prefunction

    # Define a prefunction that will be called to match the rule.
    def setPreFunction(self, function):
        self.__prefunction = function

# This is a subclass of the generic dialog rule that we use for determining whether to 
# 'include' additional rules.
class IncludeRule(DialogRule):
    def __init__(self, presemaphores):
        DialogRule.__init__(self, None, presemaphores, None, None, None )

class Dialog:
    # A character is the source that supplies keywords that drive the dialog.
    # The speaker is the NPC that responds to the keywords. A location is an
    # unique identifier that is used to distinguish dialogs from each other.
    def __init__(self, character, speaker, location):
        self.__character = character
        self.__location = location
        self.__speaker = speaker
        self.__rules = []

    # Create rules of the DialogRule class that define dialog flow. An index
    # defines the order in which rules are processed.  FIXME: addRule could
    # very easily create the index.  It is unclear why this mundane activity
    # is left for the dialog maker.
    def addRule(self, rule, index):
        self.__rules.insert(index, rule)

    # A function to call when saying something to an NPC to elicit a response
    # based on defined rules. It iterates through the rules and determines if
    # the spoken text matches a keyword.  If so, the rule preconditions and/or
    # prefunctions are checked.  If all conditions they define are met, then
    # the NPC responds, and postconditions, if any, are set.  Postfunctions
    # also execute if present.
    # some variable substitution is done on the message here, $me and $you 
    # are replaced by the names of the npc and the player respectively
    def speak(self, msg):
        # query the animation system in case the NPC is playing an animation
        if self.__speaker.Event(self.__speaker, self.__speaker, "query_object_is_animated", 1):
            return 0

        key = self.uniqueKey()
        replies = None
        if Crossfire.GetPrivateDictionary().has_key(key):
          replies = Crossfire.GetPrivateDictionary()[key]
          Crossfire.GetPrivateDictionary()[key] = None

        for rule in self.__rules:
            if self.isAnswer(msg, rule.getKeyword()) == 1:
                if self.matchConditions(rule) == 1:
                    message = rule.getMessage()
                    message = message.replace('$me', self.__speaker.QueryName())
                    message = message.replace('$you', self.__character.QueryName())

                    Crossfire.NPCSay(self.__speaker, message);
                    if rule.getSuggests() != None:
                        for reply in rule.getSuggests():
                            Crossfire.AddReply(reply[0], reply[1])
                        Crossfire.GetPrivateDictionary()[key] = rule.getSuggests()
                    self.setConditions(rule)

                    # change the player's text if found
                    if replies != None:
                        for reply in replies:
                            if reply[0] == msg:
                                type = Crossfire.ReplyType.SAY
                                if len(reply) > 2:
                                    type = int(reply[2])
                                Crossfire.SetPlayerMessage(reply[1], type)
                                break

                    return 0
        return 1

    def uniqueKey(self):
      return self.__location + '_' + self.__character.QueryName()

    # Determine if the message sent to an NPC matches a string in the keyword
    # list. The match check is case-insensitive, and succeeds if a keyword
    # string is found in the message.  This means that the keyword string(s)
    # only need to be a substring of the message in order to trigger a reply.
    def isAnswer(self, msg, keywords):
        for ckey in keywords:
            if ckey == "*" or msg.lower().find(ckey.lower()) != -1:
                return 1
        return 0

    # Check the preconditions specified in rule have been met.  Preconditions
    # are lists of one or more conditions to check.  Each condition specifies
    # a check to perform and the options it should act on.
    # separate files are used for each type of check.
    def matchConditions(self, rule):
        character = self.__character
        location = self.__location
        speaker = self.__speaker
        verdict = True
        for condition in rule.getPreconditions():
            action = condition[0]
            args = condition[1:]
            path = os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/dialog/pre/', action + '.py')
            if os.path.isfile(path):
                try:
                    exec(open(path).read())
                except:
                    Crossfire.Log(Crossfire.LogError, "CFDialog: Failed to evaluate condition %s." % condition)
                    verdict = False
                if verdict == False:
                    return 0
            else:
                Crossfire.Log(Crossfire.LogError, "CFDialog: Pre Block called with unknown action %s." % action)
                return 0

        if rule.getPreFunction() != None:
            if rule.getPreFunction()(self.__character, rule) != True:
                return 0
        return 1

    
    # If a rule triggers, this function goes through each condition and runs the file that handles it.
    def setConditions(self, rule):
        character = self.__character
        location = self.__location
        speaker = self.__speaker

        for condition in rule.getPostconditions():
            Crossfire.Log(Crossfire.LogDebug, "CFDialog: Trying to apply %s." % condition)
            action = condition[0]
            args = condition[1:]
            path = os.path.join(Crossfire.DataDirectory(), Crossfire.MapDirectory(), 'python/dialog/post/', action + '.py')
            if os.path.isfile(path):
                try:
                    exec(open(path).read())
                except:
                    Crossfire.Log(Crossfire.LogError, "CFDialog: Failed to set post-condition %s." %condition)
            else:
                Crossfire.Log(Crossfire.LogError, "CFDialog: Post Block called with unknown action %s." % action)


    # Search the player file for a particular flag, and if it exists, return
    # its value.  Flag names are combined with the unique dialog "location"
    # identifier, and are therefore are not required to be unique.  This also
    # prevents flags from conflicting with other non-dialog-related contents
    # in the player file.
    def getStatus(self, key):
        character_status=self.__character.ReadKey("dialog_"+self.__location);
        if character_status == "":
            return "0"
        pairs=character_status.split(";")
        for i in pairs:
            subpair=i.split(":")
            if subpair[0] == key:
                return subpair[1]
        return "0"

    # Store a flag in the player file and set it to the specified value.  Flag
    # names are combined with the unique dialog "location" identifier, and are
    # therefore are not required to be unique.  This also prevents flags from
    # conflicting with other non-dialog-related contents in the player file.
    def setStatus(self, key, value):
        if value == "*":
            return
        ishere = 0
        finished = ""
        character_status = self.__character.ReadKey("dialog_"+self.__location);
        if character_status != "":
            pairs = character_status.split(";")
            for i in pairs:
                subpair = i.split(":")
                if subpair[0] == key:
                    subpair[1] = value
                    ishere = 1
                if finished != "":
                    finished = finished+";"
                finished = finished + subpair[0] + ":" + subpair[1]
        if ishere == 0:
            if finished != "":
                finished = finished + ";"
            finished = finished + key + ":" + value
        self.__character.WriteKey("dialog_" + self.__location, finished, 1)

    # Search the NPC for a particular flag, and if it exists, return
    # its value.  Flag names are combined with the unique dialog "location"
    # identifier and the player's name, and are therefore are not required
    # to be unique.  This also prevents flags from conflicting with other
    # non-dialog-related contents in the NPC.
    def getNPCStatus(self, key):
        npc_status=self.__speaker.ReadKey("dialog_"+self.__location + "_" + self.__character.Name);
        if npc_status == "":
            return "0"
        pairs=npc_status.split(";")
        for i in pairs:
            subpair=i.split(":")
            if subpair[0] == key:
                return subpair[1]
        return "0"

    # Store a flag in the NPC and set it to the specified value.  Flag
    # names are combined with the unique dialog "location" identifier
    # and the player's name, and are therefore are not required to be unique.
    # This also prevents flags from conflicting with other non-dialog-related
    # contents in the player file.
    def setNPCStatus(self, key, value):
        if value == "*":
            return
        ishere = 0
        finished = ""
        npc_status = self.__speaker.ReadKey("dialog_"+self.__location + "_" + self.__character.Name);
        if npc_status != "":
            pairs = npc_status.split(";")
            for i in pairs:
                subpair = i.split(":")
                if subpair[0] == key:
                    subpair[1] = value
                    ishere = 1
                if finished != "":
                    finished = finished+";"
                finished = finished + subpair[0] + ":" + subpair[1]
        if ishere == 0:
            if finished != "":
                finished = finished + ";"
            finished = finished + key + ":" + value
        self.__speaker.WriteKey("dialog_" + self.__location + "_" + self.__character.Name, finished, 1)
