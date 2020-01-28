{
  "location" : "scorn/milton",
  "rules": [
  {
  "match" : ["no"],
  "pre" : [["quest", "scorn/PassClerk", "60"], ["token", "buy", "gate", "port", "choose"]],
  "post" : [["settoken", "buy", "0"]],
  "msg" : ["Ok then, come back later if you change your mind."]
  }, {
  "match" : ["no"],
  "pre" : [["quest", "scorn/PassClerk", "50"], ["token", "buy", "gate", "port", "choose"]],
  "post" : [["settoken", "buy", "0"]],
  "msg" : ["Then get lost."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "60"], ["item", "money", "1250"], ["token", "buy", "gate"]],
  "post" : [["settoken", "buy", "0"], ["giveitem", "Gate Pass"], ["takeitem", "money", "1250"]],
  "msg" : ["Here is your Gate Pass, $you."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "60"], ["item", "money", "5000"], ["token", "buy", "port"]],
  "post" : [["settoken", "buy", "0"], ["giveitem", "Port Pass"], ["takeitem", "money", "5000"]],
  "msg" : ["Here is your Port Pass, $you."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "50"], ["item", "money", "2500"], ["token", "buy", "gate"]],
  "post" : [["settoken", "buy", "0"], ["giveitem", "Gate Pass"], ["takeitem", "money", "2500"]],
  "msg" : ["There's yer gate pass, now leave me alone."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "50"], ["item", "money", "10000"], ["token", "buy", "port"]],
  "post" : [["settoken", "buy", "0"], ["giveitem", "Port Pass"], ["takeitem", "money", "10000"]],
  "msg" : ["There's yer port pass, now leave me alone."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "60"], ["token", "buy", "gate", "port"]],
  "post" : [["settoken", "buy", "0"]],
  "msg" : ["You can't afford the pass, $you."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "50"], ["token", "buy", "gate", "port"]],
  "post" : [["settoken", "buy", "0"]],
  "msg" : ["If you're going to go to the trouble of ordering a pass, at least have the money to pay for it. \nCome back when you have enough.\n Better still, don't come back at all."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"], ["item", "Eswald's superior coffee", "1"]],
  "post" : [["quest", "scorn/PassClerk", "60"], ["settoken", "buy", "choose"], ["takeitem", "Eswald's superior coffee", "1"]],
  "msg" : ["<SLURP>.... Wow, that is seriously good coffee, thank you friend, now, were you after a pass?"]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"], ["item", "Oled's coffee"]],
  "post" : [["quest", "scorn/PassClerk", "50"], ["settoken", "buy", "choose"], ["takeitem", "Oled's coffee"]],
  "msg" : ["<SLURP>.... hmm, that coffee will suffice, I guess now. Now I suppose you'll be wanting a pass?"]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"], ["item", "Eswald's inferior coffee"], ["quest", "scorn/CoffeeGoose", "10"]],
  "post" : [["quest", "scorn/PassClerk", "40"], ["settoken", "buy", "choose"], ["takeitem", "Eswald's inferior coffee", "1"]],
  "msg" : ["<SLURP>...<Cough>...<SPIT> What are you trying to do, poison me! That coffee is terrible, you'll need to find something a lot  better than that."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"], ["item", "Eswald's inferior coffee"]],
  "post" : [["quest", "scorn/PassClerk", "40"], ["settoken", "buy", "choose"], ["takeitem", "Eswald's inferior coffee", "1"], ["quest", "scorn/CoffeeGoose", "15"]],
  "msg" : ["<SLURP>...<Cough>...<SPIT> What are you trying to do, poison me! That coffee is terrible, the worst I have ever tasted, you'll need to find something a lot better than that."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"], ["item", "cup of coffee"]],
  "post" : [["settoken", "buy", "0"]],
  "msg" : ["No, that's no good, I won't drink anything other than local, freshly brewed coffee."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "30"], ["token", "buy", "coffee"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Well, where is it then? Stop wasting my time and get me my coffee, I have important ... er ... things to be doing here you know."]
  }, {
  "match" : ["port"],
  "pre" : [["quest", "scorn/PassClerk", "60"]],
  "post" : [["settoken", "buy", "port"]],
  "msg" : ["A port pass would normally be 200 platinum, but, because it's you, I can sell you one for 100, do you want a port pass?"]
  }, {
  "match" : ["gate"],
  "pre" : [["quest", "scorn/PassClerk", "60"]],
  "post" : [["settoken", "buy", "gate"]],
  "msg" : ["A gate pass should be 50 platinum, but, because it's you, I'll sell you one for 25, do you want a gate pass?"]
  }, {
  "match" : ["port"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [["settoken", "buy", "port"]],
  "msg" : ["Port passes are 200 platinum, Do you want one?"]
  }, {
  "match" : ["gate"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [["settoken", "buy", "gate"]],
  "msg" : ["Gate passes are 50 platinum, Do you want one?"]
  }, {
  "match" : ["coffee"],
  "pre" : [["quest", "scorn/PassClerk", "30"]],
  "post" : [["settoken", "buy", "coffee"]],
  "msg" : ["Have you got my coffee yet?"]
  }, {
  "match" : ["coffee"],
  "pre" : [],
  "post" : [["quest", "scorn/PassClerk", "30"]],
  "msg" : ["Well, if you get me some coffee I might be able to think about serving you... maybe.\n But you'd better get some local coffee, not any of that dodgy foreign muck, and it had better be freshly made, by hand; I don't trust that magick coffee the summoners keep making."]
  }, {
  "match" : ["closed"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [],
  "msg" : ["Normally we only open on alternate mornings on Days of the Bull upon presentation of a correctly authorised form ZZ9 completed in triplicate and counter-signed by a duly authorised signatory on the Day of the Deception the previous week. Said signature to have been witnessed to by a member of the Office of Passes whose presence must have been requested at least two months previously via the submission of form P46 through the office of the magistrate.... However, I guess in your case I can make an exception."]
  }, {
  "match" : ["closed"],
  "pre" : [],
  "post" : [],
  "msg" : ["Yes, closed, as in leave me alone and go bother someone else."]
  }, {
  "match" : ["pass"],
  "pre" : [["quest", "scorn/PassClerk", "60"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Would you like a port pass or a gate pass?"]
  }, {
  "match" : ["pass"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Port pass or gate pass?"]
  }, {
  "match" : ["pass", "gate", "port"],
  "pre" : [],
  "post" : [],
  "msg" : ["Yes, we have passes and, no, I'm not selling you one, go away."]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "60"], ["token", "buy", "choose"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Would you like a port pass or a gate pass?"]
  }, {
  "match" : ["yes"],
  "pre" : [["quest", "scorn/PassClerk", "50"], ["token", "buy", "choose"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Port pass or gate pass?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/PassClerk", "60"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["Good day to you $you, would you like to buy a pass?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/PassClerk", "50"]],
  "post" : [["settoken", "buy", "choose"]],
  "msg" : ["oh, hello, I suppose you'll be wanting a pass then?"]
  }, {
  "match" : ["*"],
  "pre" : [["quest", "scorn/PassClerk", "10"]],
  "post" : [],
  "msg" : ["I told you, we're closed, go away and leave me alone."]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [["quest", "scorn/PassClerk", "10"]],
  "msg" : ["*Yawn* Arrrhh, what do you want? We're closed!"]
  }
]}
