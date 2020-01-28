{
  "rules": [
  {
  "match" : ["curse", "calamity", "spectre", "yes"],
  "pre" : [["quest", "scorn/LordSpectre", "10"], ["item", "Calamity"], ["token", "question", "curse"]],
  "post" : [["takeitem", "Calamity"], ["quest", "scorn/LordSpectre", "60"], ["givecontents", "Prince Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest. We have a new Prince of the Realm!\nPlease take this fine sword as a reward.  May it serve you well!"]
  }, {
  "match" : ["curse", "calamity", "spectre", "yes"],
  "pre" : [["quest", "scorn/LordSpectre", "10"], ["token", "question", "curse"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/LordSpectre", "10"]],
  "post" : [["settoken", "question", "curse"]],
  "msg" : ["Have you returned with a means of lifting the curse?"],
  "replies" : [["no", "No, Your Majesty, I am still searching"], ["yes", "Yes Your Majesty, I have found the means to lift your curse"]]
  }, {
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  }, {
  "match" : ["yes", "service", "Scorn", "help"],
  "pre" : [],
  "post" : [["quest", "scorn/LordSpectre", "10"]],
  "msg" : ["Come close, and listen carefully, I don't want my retainers overhearing...\n I have for some time now been afflicted by an ailment which has been weakening me. My physicians proved unable to cure it, so I turned to the high priests. The devotee of Gaea claims to have had the cause revealed to her, speak to her at Gaea's temple here in town, and bring me the means to break this curse."]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Lord $you, would you be prepared to help me once more?"],
  "replies" : [["no", "No, Your Majesty"], ["yes", "Yes, Your Majesty"]]
  }
]}
