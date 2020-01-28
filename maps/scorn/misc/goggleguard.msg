{
  "location" : "scorn/alfalfa",
  "rules": [
  {
  "match" : ["*"],
  "pre" : [["quest", "scorn/OrcDress", "40"]],
  "post" : [],
  "msg" : ["I wagered two week's pay to secure this post, and now she's gone and ruined the view."]
  },{
  "match" : ["clothes"],
  "pre" : [],
  "post" : [],
  "msg" : ["Give her clothes, and spoil the view! Not likely."],
  "replies" : [["view", "What view is that?"]]
  },{
  "match" : ["post"],
  "pre" : [],
  "post" : [],
  "msg" : ["This post has become the most sought after post amongst the guards. It has the best view in the prison!"],
  "replies": [["view", "What's so good about the view?"]]
  },{
  "match" : ["view"],
  "pre" : [],
  "post" : [],
  "msg" : ["Can't you see the great looking lass across the corridor? Such long legs and so big too."]
  },{
  "match" : ["*"],
  "pre" : [],
  "post" : [],
  "msg" : ["I won this post from the other guards in a game. Now go away and let me watch the great view."],
  "replies" : [["view", "What view are you watching?"], ["post", "Why'd you want to win this post?"]]
  }
]}
