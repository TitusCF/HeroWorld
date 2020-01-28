{
  "location" : "scorn_terrysfarm",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "70"]],
  "post" : [],
  "msg" : ["You murdered my husband! Fly from here foul agent of Guuh."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "60"]],
  "post" : [],
  "msg" : ["Thank you so much for saving us from the evil bandits"]
  }, {
  "match" : ["no"],
  "pre" : [["token", "highlevel", "head"], ["quest", "scorn/TerrysFarm", "50"]],
  "post" : [["settoken", "highlevel", "0"]],
  "msg" : ["You'll need to bring back his head as proof that you have killed him."]
  }, {
  "match" : ["yes"],
  "pre" : [["token", "highlevel", "head"], ["quest", "scorn/TerrysFarm", "50"], ["item", "Guuh's head"]],
  "post" : [["takeitem", "Guuh's head", "1"], ["quest", "scorn/TerrysFarm", "60"], ["connection", "4"]],
  "msg" : ["Thank you so much for saving us from the evil bandits, please help yourself to whatever you wish from our treasure room"]
  }, {
  "match" : ["yes"],
  "pre" : [["token", "highlevel", "head"], ["quest", "scorn/TerrysFarm", "50"]],
  "post" : [["settoken", "highlevel", "0"]],
  "msg" : ["Bring his head here and I will reward you"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "50"]],
  "post" : [["settoken", "highlevel", "head"]],
  "msg" : ["You've killed the Bandit Leader Guuh? have you recovered his head?"],
  "replies" : [["yes", "Yes, I killed him.", "1"], ["no", "Unfortunately no, not yet.", "0"]]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TerrysFarm", "=40"]],
  "post" : [],
  "msg" : ["So did you kill Ghuu? Oh please, help us, we can't do anything while he's here!"]
  }, {
  "match" : ["yes"],
  "pre" : [["token", "highlevel", "yes"]],
  "post" : [["quest", "scorn/TerrysFarm", "40"], ["giveitem", "Farmer's key"]],
  "msg" : ["Thank you mighty adventurer. Here, take this key, it will open the door to the room they have occupied. Bring me the head of the bandit leader Guuh and I will reward you."]
  }, {
  "match" : ["yes"],
  "pre" : [["token", "highlevel", "maybe"], ["token", "lethelp", "yes"]],
  "post" : [["quest", "scorn/TerrysFarm", "30"], ["giveitem", "Farmer's key"]],
  "msg" : ["Very well then, take this key, it will open the door to the room that Guuh and his band have occupied. Bring me Guuh's head and I will reward you. \n And please do be careful"]
  }, {
  "match" : ["no"],
  "pre" : [["token", "highlevel", "yes"]],
  "post" : [["settoken", "highlevel", "0"]],
  "msg" : ["I understand mighty adventurer, someone as powerful as you will have more important things to worry about than the fate of some poor farmers. Oh woe are we."]
  }, {
  "match" : ["no"],
  "pre" : [["token", "highlevel", "maybe"]],
  "post" : [["settoken", "highlevel", "0"]],
  "msg" : ["A wise choice adventurer, if we let someone in to fight Guuh, we must know that he will be victorious otherwise Guuh may win and then escape to cause more havoc."]
  }, {
  "match" : ["strong", "help", "bandit", "guuh"],
  "pre" : [["token", "highlevel", "maybe"]],
  "post" : [["settoken", "lethelp", "yes"]],
  "msg" : ["Guuh and his band are probably too strong for you to defeat in combat, but if you want to take them on, I won't stop you. Do you want to fight Guuh?"],
  "replies" : [["yes", "Yes, definitely.", "1"], ["no", "Hum, after some thinking, no.", "1"]]
  }, {
  "match" : ["*"],
  "pre" : [["level", "7"]],
  "post" : [["settoken", "highlevel", "yes"]],
  "msg" : ["Hello mighty adventurer, we need your help.\nA group of bandits led by Guuh have invaded our farmhouse and occupied the room to the east. will you help us by ridding us of these evil bandits?"],
  "replies" : [["yes", "Yes, of course.", "1"], ["no", "I'm sorry, but not.", "0"]]
  }, {
  "match" : ["*"],
  "pre" : [["level", "3"]],
  "post" : [["settoken", "highlevel", "maybe"]],
  "msg" : ["Hello adventurer, A group of bandits led by Guuh have invaded our farmhouse and occupied the room to the east. I would ask you to help, but I'm not sure you are strong enough."],
  "replies" : [["help", "It is my duty to help.", "0"], ["strong", "Well, how can I become stronger if I don't fight harder foes?", "2"]]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [["settoken", "highlevel", "no"], ["quest", "scorn/TerrysFarm", "20"]],
  "msg" : ["Hello adventurer, I'm afraid I can't chat, we have a load of bandits locked up in the room to the east, if you were stronger I might ask you to help, but if I asked for your assistance now, then you would be slaughtered by the evil bandits."]
  }
]}
