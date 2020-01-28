{
  "location" : "scorn/smith",
  "rules": [
  {
  "comment": "This file handles the giving of the finished items to the player",
  "include" : ["smith_finished.msg"],
  "pre" : [["age", "orderplaced", "0", "0", "1", "0", "0"]]
  },

  { 
  "comment": "Player returned too soon, patience padawan.",
  "match" : ["*"],
  "pre" : [["token", "ordered", "eyeshield", "redshield", "redmail", "bluemail", "greenmail"]],
  "post": [],
  "msg" : ["I'm sorry, your order isn't ready yet, please come back later."]
  },


  {
  "comment": "Actually ordering the items",
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredmail"], ["item", "red dragon scale", "5"],["item", "money", "10000"]],
  "post": [["settoken", "orderstatus", "0"], ["settoken", "ordered", "redmail"],["takeitem", "red dragon scale", "5"],["takeitem", "money", "10000"],["quest", "scorn/smith/customarmour","30"], ["marktime", "orderplaced"]],
  "msg" : ["Thank you, your red dragon mail will be ready by this time tomorrow."]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buygreenmail"], ["item", "green dragon scale", "5"],["item", "money", "10000"]],
  "post": [["settoken", "orderstatus", "0"], ["settoken", "ordered", "greenmail"],["takeitem", "green dragon scale", "5"],["takeitem", "money", "10000"],["quest", "scorn/smith/customarmour","30"], ["marktime", "orderplaced"]],
  "msg" : ["Thank you, your green dragon shield will be ready by this time tomorrow."]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buybluemail"], ["item", "blue dragon scale", "5"],["item", "money", "10000"]],
  "post": [["settoken", "orderstatus", "0"], ["settoken", "ordered", "bluemail"],["takeitem", "blue dragon scale", "5"],["takeitem", "money", "10000"],["quest", "scorn/smith/customarmour","30"], ["marktime", "orderplaced"]],
  "msg" : ["Thank you, your blue dragon shield will be ready by this time tomorrow."]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredshield"], ["item", "red dragon scale"],["item", "money", "5000"]],
  "post": [["settoken", "orderstatus", "0"], ["settoken", "ordered", "redshield"],["takeitem", "red dragon scale"],["takeitem", "money", "5000"],["quest", "scorn/smith/customarmour","20"], ["marktime", "orderplaced"]],
  "msg" : ["Thank you, your dragon shield will be ready by this time tomorrow."]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyeye"], ["item", "beholdereye"],["item", "money", "2500"]],
  "post": [["settoken", "orderstatus", "0"], ["settoken", "ordered", "eyeshield"],["takeitem", "beholdereye"],["takeitem", "money", "2500"],["quest", "scorn/smith/customarmour","10"], ["marktime", "orderplaced"]],
  "msg" : ["Thank you, your eye shield will be ready by this time tomorrow."]
  },


  {
  "comment": "The player wants to buy the item, but can't afford it",
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredshield"], ["item", "red dragon scale"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough money to pay for the dragon shield, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyeye"], ["item", "beholdereye"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough money to pay for the eye shield, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buybluemail"], ["item", "blue dragon scale", "5"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough money to pay for the dragon mail, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredmail"], ["item", "red dragon scale", "5"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough money to pay for the dragon mail, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buygreenmail"], ["item", "green dragon scale", "5"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough money to pay for the dragon mail, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredmail", "buybluemail", "buygreenmail"], ["item", "money", "10000"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have enough dragon scales for me to make the dragon mail that you want, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredshield"], ["item", "money", "5000"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you will need to have a red dragon scale in order to pay for the dragon shield, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyeye"], ["item", "money", "2500"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you'll need to have a beholdereye in order for me to make you an eye shield, would you like to order something else instead?"]
  },{
  "match" : ["yes"],
  "pre" : [["token", "orderstatus", "buyredshield", "buyeye", "buyredmail", "buygreenmail", "buybluemail"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["I'm sorry, you don't have either the right materials or enough money for me to make that item for you, would you like to order something else instead?"]
  },


{ "comment": "No Sale, Try going back to the goods list",
  "match" : ["no"],
  "pre" : [["token", "orderstatus", "buyredshield", "buyeye", "buyredmail", "buygreenmail", "buybluemail"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["Is there something else I can interest you in?"]
  },


{ "comment": "Selling dragon mail",
  "match" : ["red", "fire"],
  "pre" : [["token", "orderstatus", "choosemail"]],
  "post": [["settoken", "orderstatus", "buyredmail"]],
  "msg" : ["To make red dragon mail I will need 5 red dragon scales plus 200 platinum to cover my expenses. Would you like me to make you some red dragon mail?"]
  },{
  "match" : ["green", "cold"],
  "pre" : [["token", "orderstatus", "choosemail"]],
  "post": [["settoken", "orderstatus", "buygreenmail"]],
  "msg" : ["To make green dragon mail I will need 5 green dragon scales plus 200 platinum to cover my expenses. Would you like me to make you some green dragon mail?"]
  },{
  "match" : ["blue", "lightning"],
  "pre" : [["token", "orderstatus", "choosemail"]],
  "post": [["settoken", "orderstatus", "buybluemail"]],
  "msg" : ["To make blue dragon mail I will need 5 blue dragon scales plus 200 platinum to cover my expenses. Would you like me to make you some blue dragon mail?"] },
{
  "match" : ["else", "something", "no"],
  "pre" : [["token", "orderstatus", "choosemail", "chooseshield"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["Can I interest you in any of my other armour then?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "orderstatus", "choosemail"]],
  "post": [],
  "msg" : ["I'm sorry, I don't understand what you are asking for, are you after red, green or blue dragon mail, or would you like something else entirely?"]
  },{
  "match" : ["mail"],
  "pre" : [["token", "orderstatus", "choosegoods"]],
  "post": [["settoken", "orderstatus", "choosemail"]],
  "msg" : ["I can make one of three different kinds of dragon mail, depending on the type of dragon scales you can give me to make it. There is red dragon mail, which protects against fire, green dragon mail, which protects against cold or blue dragon mail which protects against lightning, which would you like?"]
  },


{ "comment": "Selling Shields",
  "match" : ["dragon"],
  "pre" : [["token", "orderstatus", "chooseshield"]],
  "post": [["settoken", "orderstatus", "buyredshield"]],
  "msg" : ["The dragon shield is both a solid piece of defense equipment and also offers protection against fire. I can make you one with a red dragon scale plus 100 platinum to cover my expenses. Would you like me to make you an dragon shield?"]
  },{
  "match" : ["dragon shield"],
  "pre" : [["token", "orderstatus", "choosegoods", "interest"]],
  "post": [["settoken", "orderstatus", "buyredshield"]],
  "msg" : ["The dragon shield is both a solid piece of defense equipment and also offers protection against fire. I can make you one with a red dragon scale plus 100 platinum to cover my expenses. Would you like me to make you an dragon shield?"]
  },{
  "match" : ["eyeshield", "eye"],
  "pre" : [["token", "orderstatus", "choosegoods", "chooseshield", "interest"]],
  "post": [["settoken", "orderstatus", "buyeye"]],
  "msg" : ["The eyeshield offers very good protection against any dangerous magic you might encounter on your travels. I can make you one if you can provide a beholder's eye plus 50 platinum to cover my expenses. Would you like me to make you an eyeshield?"]
  },{
  "match" : ["shield"],
  "pre" : [["token", "orderstatus", "choosegoods", "interest"]],
  "post": [["settoken", "orderstatus", "chooseshield"]],
  "msg" : ["Is it an eyeshield or a dragon shield that you interested in?"]
  },{
  "match" : ["*"],
  "pre" : [["token", "orderstatus", "chooseshield"]],
  "post": [],
  "msg" : ["I'm sorry, I don't understand what you are asking for, are you after an eyeshield, a dragon shield or would you like something else entirely?"]
  },



{
  "comment" : "Allow players to ask extra information up front.",
  "match" : ["cost", "money", "gold", "platinum", "silver", "expenses"],
  "pre" : [["token", "orderstatus", "interest", "choosegoods"]],
  "post": [["settoken", "orderstatus", "choosegoods"]],
  "msg" : ["My expenses will depend on the item you want me to make, for an eyeshield, it is 50 platinum, for a dragon shield 100 platinum, for dragon mail it is 200 platinum, what would you like me to make for you?"]
  },{
  "match" : ["species", "colour", "color"],
  "pre" : [["token", "orderstatus", "interest", "choosegoods"]],
  "post": [["settoken", "orderstatus", "choosegoods"]],
  "msg" : ["I can use the scales of either red, green or blue dragons to make dragon mail, which will grant the wearer protection from fire, cold or lightning respectively."]
  },{
  "match" : ["materials", "ingredients"],
  "pre" : [["token", "orderstatus", "interest", "choosegoods"]],
  "post": [["settoken", "orderstatus", "choosegoods"]],
  "msg" : ["In order to make an eyeshield, I will need a beholder's eye, to make a dragon shield, I will need the scale of a red dragon, and in order to make a dragon mail, I will need 5 scales from the species of dragon you want me to make the mail from.?"]
  },{


  "comment" : "list options, get the player to ask for what they want.",
  "match" : ["custom", "armour", "armor", "yes"],
  "pre" : [["token", "orderstatus", "interest"]],
  "post": [["settoken", "orderstatus", "choosegoods"]],
  "msg" : ["If you have both the raw materials, and the money to cover my expenses, then I can make you an eyeshield, a dragon shield, or a suit of dragon mail, what are you interested in?"]
  },{
  "match" : ["no"],
  "pre" : [["token", "orderstatus", "interest"]],
  "post": [["settoken", "orderstatus", "0"]],
  "msg" : ["Ok then, come back later if you change your mind."]
  

},{
  "comment" : "start of conversation, set order status to 'interest'",
  "match" : ["*"],
  "pre" : [["token", "Seen before", "yes"]],
  "post": [["settoken", "orderstatus", "interest"]],
  "msg" : ["Hello again, $you, can I interest you in some custom armour?"]    
  },{
  "match" : ["*"],
  "pre" : [],
  "post": [["settoken", "orderstatus", "interest"],["settoken", "Seen before", "yes"]],
  "msg" : ["Hello there, can I interest you in some custom armour?"]    
  }
]}