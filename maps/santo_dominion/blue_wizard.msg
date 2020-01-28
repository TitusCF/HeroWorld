{
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["npctoken", "done", "1"]],
  "post" : [],
  "msg" : ["Hello, adventurer.\n\nPlease come back later, then I may have something for you."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "santo_dominion/Mudman", "10-30"], ["npctoken", "dlg", "ask_corpse"], ["item", "Mudman's residue"]],
  "post" : [["setnpctoken", "dlg", "0"], ["setnpctoken", "done", "1"], ["quest", "santo_dominion/Mudman", "40"], ["connection", "1"], ["takeitem", "Mudman's residue"]],
  "msg" : ["Congratulations!\n\nYou can take the treasure."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "santo_dominion/Mudman", "10-30"], ["npctoken", "dlg", "ask_corpse"]],
  "post" : [["setnpctoken", "dlg", "0"]],
  "msg" : ["Where is the body, then?"]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "santo_dominion/Mudman", "10-30"], ["npctoken", "dlg", "ask_corpse"]],
  "post" : [["setnpctoken", "dlg", "0"]],
  "msg" : ["Well, please come back when you have it, then."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "santo_dominion/Mudman", "10-30"]],
  "post" : [["setnpctoken", "dlg", "ask_corpse"]],
  "msg" : ["Did you kill the Mudman?"],
  "replies" : [["yes", "Yes.", "1"], ["no", "No, not yet.", "1"]]
  }, {
  "match" : ["yes"],
  "pre" : [["npctoken", "dlg", "ask_quest"]],
  "post" : [["setnpctoken", "dlg", "0"], ["quest", "santo_dominion/Mudman", "10"]],
  "msg" : ["Then bring me the residue."]
  }, {
  "match" : ["*"],
  "pre" : [["npctoken", "dlg", "ask_quest"]],
  "post" : [["setnpctoken", "dlg", "0"]],
  "msg" : ["Ha well, come back if you want to try your strength."]
  }, {
  "match" : ["mudman"],
  "pre" : [["npctoken", "dlg", "ask_mudman"]],
  "post" : [["setnpctoken", "dlg", "ask_quest"]],
  "msg" : ["The Mudman is a vile creature lurking in swamps just west of this town.\nThe inhabitants of St. Dominion are scared of the Mudman. Kill him and put his residue on my table. Then I will open the gate for you.\n\nSo are you ready for this challenge?"],
  "replies" : [["yes", "Sure, I'm more powerful than I appear!", "1"], ["no", "Well, not now, I feel lazy.", "1"]]
  }, {
  "match" : ["yes"],
  "pre" : [["npctoken", "dlg", "ask"]],
  "post" : [["setnpctoken", "dlg", "ask_mudman"]],
  "msg" : ["Do you see this room behind me? It's the \"hero's room\". You may enter if you prove yourself worthy by killing the Mudman."],
  "replies" : [["mudman", "What is the mudman?", "2"]]
  }, {
  "match" : ["*"],
  "pre" : [["npctoken", "dlg", "ask"]],
  "post" : [["setnpctoken", "dlg", "0"]],
  "msg" : ["Well, maybe another time, then."]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [["setnpctoken", "dlg", "ask"]],
  "msg" : ["Hello adventurer.\n\nDo you want a challenge?"],
  "replies" : [["yes", "Yes, of course, I'm always ready for challenges.", "1"]]
  }
]}
