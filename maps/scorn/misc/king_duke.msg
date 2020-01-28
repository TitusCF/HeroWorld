{
  "rules": [
  {
  "match" : ["titan", "crown", "pretender", "meganthropopulos", "yes"],
  "pre" : [["quest", "scorn/TitanCrown", "10"], ["item", "Meganthropopulos' Crown"], ["token", "question", "crown"]],
  "post" : [["takeitem", "Meganthropopulos' Crown"], ["quest", "scorn/TitanCrown", "80"], ["givecontents", "Archduke Reward"], ["settoken", "question", "0"]],
  "msg" : ["Congratulations on completing your quest.  Kneel and receive your accolade.....\n You are now an Archduke! \n Also, please take this fine shield as a reward. May it serve you well!"]
  }, {
  "match" : ["titan", "crown", "pretender", "meganthropopulos", "yes"],
  "pre" : [["quest", "scorn/TitanCrown", "10"], ["token", "question", "crown"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["Where is it then?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TitanCrown", "10"]],
  "post" : [["settoken", "question", "crown"]],
  "msg" : ["Have you returned with the pretender's crown?"],
  "replies" : [["no", "No, your majesty, I still seek the crown"], ["yes", "Yes, Your majesty, I have the Titan's crown"]]
  }, {
  "match" : ["no"],
  "pre" : [],
  "post" : [],
  "msg" : ["Very well then Lord $you, do return here if you change your mind."]
  }, {
  "match" : ["yes", "service", "Scorn"],
  "pre" : [],
  "post" : [["quest", "scorn/TitanCrown", "10"]],
  "msg" : ["A wayward Titan has been styling himself 'the Rightful King of Scorn' and named me an Usurper.\n Clearly this insult demands a suitably swift and decisive response.\n This titan has created a crown for himself, deprive him of it, if you choose to deprive him of his life at the same time, then I would not be displeased.\n Speak to my magistrate, he may know where the pretender hides"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hail Lord $you, would you like to be of service to your King?"],
  "replies" : [["no", "No, Your Majesty"], ["yes", "Yes, Your Majesty"]]
  }
]}
