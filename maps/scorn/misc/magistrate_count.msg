{
  "rules": [
  {
  "match" : ["dragon", "weapon", "look", "yes", "assistance"],
  "pre" : [["quest", "scorn/DragonWhisker", "10"], ["token", "question", "helpdragon"]],
  "post" : [["settoken", "question", "0"], ["quest", "scorn/DragonWhisker", "20"]],
  "msg" : ["So you are seeking the Dragonlords? They did send an emissary to the court a while back, but he hasn't been seen around here very often.\n From what I hear, he seems to be something of a wastrel, and spends almost all of his time in the taverns. You may be able to make use of his lack of moral fortitude to <ahem> persuade him to tell you more than he should."]
  },{
  "match" : ["no"],
  "pre" : [["token", "question", "helpdragon"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["My apologies for the impertience My Lord, I am but your humble servant."]
  },
    {
  "match" : ["*"],
  "pre" : [["quest", "scorn/DragonWhisker", "10"]],
  "post" : [["settoken", "question", "helpdragon"]],
  "msg" : ["May I be of assistance, My Lord $you?"],
  "replies" : [["yes", "Yes, you may, tell me of the Dragonlords"], ["no", "No, I need no assistance from you"]]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["My Lord $you, I am unworthy to be in your presence, please, go upstairs and speak to the King directly."]
  }
]}