{
  "rules": [
  {
  "match" : ["titan", "pretender", "crown", "yes", "assistance"],
  "pre" : [["quest", "scorn/TitanCrown", "10"], ["token", "question", "helptitan"]],
  "post" : [["settoken", "question", "0"], ["quest", "scorn/TitanCrown", "20"]],
  "msg" : ["So the King wishes you to depose the titan, Meganthropopulos? This will be no easy task I fear.\n We know that Meganthropopulos has secured himself in a stronghold on the extreme south-east of this continent, we can not launch a direct assault on him though, because this stronghold is well defended and located on a narrow peninsula with only one approach.\n You may be able to sneak in and accomplish the task however.\n Good Luck Your Grace."]
  }, {
  "match" : ["no"],
  "pre" : [["token", "question", "helptitan"]],
  "post" : [["settoken", "question", "0"]],
  "msg" : ["My apologies for the impertience Your Grace, I am but your humble servant."]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/TitanCrown", "10"]],
  "post" : [["settoken", "question", "helptitan"]],
  "msg" : ["May I be of assistance, My Lord $you?"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["My Lord $you, I am unworthy to be in your presence, please, go upstairs and speak to the King directly."]
  }
]}
