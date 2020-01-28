{
  "location" : "scorn/oled",
  "rules": [
  {
  "match" : ["hello", "hi"],
  "pre" : [["token", "seenbefore", "yes"]],
  "post" : [],
  "msg" : ["Hello $you, would you like some coffee?"]
  }, {
  "match" : ["hello", "hi"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hello stranger, would you like some coffee?."]
  }, {
  "match" : ["lady"],
  "pre" : [["token", "seenbefore", "yes"]],
  "post" : [],
  "msg" : ["I think her name was something like Jann or Ann...\nworks over at the 'Mule sometimes."]
  }, {
  "match" : ["amy"],
  "pre" : [],
  "post" : [],
  "msg" : ["Hey, whatever she told you is a lie ok.\nThat lady Mark was talking to was a sick friend of mine."]
  }, {
  "match" : ["mark"],
  "pre" : [],
  "post" : [],
  "msg" : ["Who?  Never heard of him..."]
  }, {
  "match" : ["Eswald"],
  "pre" : [["quest", "scorn/CoffeeGoose", "60"]],
  "post" : [],
  "msg" : ["That skunk Eswald and his stupid goose, now that he's got it back, they are undercutting my prices again."]
  }, {
  "match" : ["Eswald"],
  "pre" : [["quest", "scorn/CoffeeGoose", "20"]],
  "post" : [],
  "msg" : ["You say that Eswald has lost his goose? That's fantastic news; they've been undercutting my prices for years. \n\n er... I mean oh dear, what a shame, I am sorry to hear that."]
  }, {
  "match" : ["Eswald"],
  "pre" : [],
  "post" : [],
  "msg" : ["That skunk Eswald and his stupid goose Undercutting my prices.  One day..."]
  }, {
  "match" : ["no"],
  "pre" : [["token", "offercoffee", "yes"]],
  "post" : [["settoken", "offercoffee", "0"]],
  "msg" : ["No problem friend, come back later if you change your mind."]
  }, {
  "match" : ["yes", "like", "cup"],
  "pre" : [["token", "offercoffee", "yes"], ["item", "money", "3"]],
  "post" : [["giveitem", "Oled's coffee"], ["settoken", "offercoffee", "0"], ["takeitem", "money", "3"]],
  "msg" : ["Here you go, "]
  }, {
  "match" : ["yes", "like", "cup"],
  "pre" : [["token", "offercoffee", "yes"]],
  "post" : [["settoken", "offercoffee", "0"]],
  "msg" : ["You'll need money to pay for the coffee, friend."]
  }, {
  "match" : ["coffee", "fresh", "cup"],
  "pre" : [],
  "post" : [["settoken", "offercoffee", "yes"]],
  "msg" : ["Coffee is 3 silver a cup, would you like some?"]
  }, {
  "match" : ["*"],
  "pre" : [],
  "post" : [["settoken", "seenbefore", "yes"]],
  "msg" : ["Have a cup of coffee! Fresh coffee!"]
  }
]}
