# -*- coding: utf-8 -*-
# animate.py
# This is one of the files that can be called by an npc_dialog.
# The following code runs when a dialog has a post rule of 'animate'
# The syntax is ["animate", "path to the animation file", "animation"]
# Have the NPC perform the specified animation.
# Path is mandatory, "animation" defaults to the animation specified
# in the file by the "animation" tag.
# During the animation, the NPC will not reply to dialogs at all.
## DIALOGCHECK
## MINARGS 1
## MAXARGS 2
## .*
## .*
## ENDDIALOGCHECK

event = speaker.CreateObject("event_user")
event.Title = "Animator"
event.Slaying = args[0]
anim = ""
if len(args) > 1:
    anim = args[1]
speaker.Event(speaker, speaker, anim, 0)
