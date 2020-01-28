{
  "rules": [
  {
  "match" : ["dragon", "whisker", "lords", "weapon", "yes"],
  "pre" : [["quest", "scorn/DragonWhisker", "10"], ["item", "Ruggilli's Whisker"], ["token", "question", "whisker"]],
  "post" : [["takeitem", "Ruggilli's Whisker"], ["quest", "scorn/DragonWhisker", "70"], ["givecontents", "Duke Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now a Duke!\n Also, please take this fine sword as a reward. May it serve you well, the NEXT time you run into dragons."]
  },{
  "match" : ["dragon", "whisker", "lords", "weapon", "yes"],
  "pre" : [["quest", "scorn/DragonWhisker", "10"], ["token", "question", "whisker"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  },{
  "match" : ["*"],
  "pre" : [["quest", "scorn/DragonWhisker", "10"]],
  "post" : [["settoken", "question", "whisker"]],
  "msg" : ["Have you returned with Ruggilli's Whisker?"],
  "replies": [["no", "No, Your Majesty, I am still looking for it"], ["yes", "Yes, Your majesty, I have found the weapon you need to bring the Dragonlords to heel"]]
  },{
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  },{
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/DragonWhisker", "10"]],
  "msg" : ["I am concerned by the threat of war from the Dragonlords.\n What I need is some bargaining leverage.\n The Dragonlords have captured a mighty weapon that could be used against them. They would destroy it, but lack the power to do so. Obtain this weapon for me and they will be intimidated into maintaining the peace. My magistrates might know where you should start looking."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Lord $you, would you like to be of service to your King?"],
  "replies": [["no", "No, Your Majesty"], ["yes", "Yes, Your Majesty"]]
  }
]}