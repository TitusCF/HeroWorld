{
  "location": "scorn/gaea",
  "rules": [
  {
  "match" : ["force"],
  "pre" : [],
  "post" : [],
  "msg" : ["If you cannot feel it, worship!  Enjoy the life that wells within Her children, and the power she bestows them to oppose all that is unnatural!"]
  },{
  "match" : ["gaea", "mother"],
  "pre" : [],
  "post" : [],
  "msg" : ["We are the children of Gaea.  Kneel down and pray to your Mother.  Feel Her force under your feet."],
  "replies" : [["force", "What force does Gaea have?"]]
  },{
  "match" : ["yes", "king", "curse"],
  "pre" : [["quest", "scorn/LordSpectre", "=10"]],
  "post" : [],
  "msg" : ["The King has been cursed. At midnight on the Dark Day last year, A Lord Spectre forged a foul attachment to The King, to his very soul. This spectre has been drawing forth a portion of his strength in order to empower a mighty weapon.\n\n A binding such as this can be undone, but only if the King gains possession of this object.\n\n You will need to go and find the Lord Spectre and destroy him, and return the weapon to the King."],
  "replies" : [["spectre", "Where may I find this foul spectre?"]]
  },{
  "match" : ["spectre"],
  "pre" : [["quest", "scorn/LordSpectre", "=10"]],
  "post" : [["quest", "scorn/LordSpectre", "20"]],
  "msg" : ["The entrance to the warren in which the Lord Spectre resides may be found on the Pirate Island."]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/LordSpectre", "=10"]],
  "post" : [],
  "msg" : ["I am celebrating Gaea, our Mother. Has the king sent you?"],
  "replies" : [["Gaea", "Gaea is your mother?"], ["yes", "Yes, the king sent me to learn about his illness"], ["no", "No, I just wanted to have a look around"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["I am celebrating Gaea, our Mother."],
  "replies" : [["Gaea", "Gaea is your mother?"]]
  }
]}
