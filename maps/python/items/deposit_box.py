import CFBank
import Crossfire

activator = Crossfire.WhoIsActivator()
whoami = Crossfire.WhoAmI()

def get_inventory(obj):
    """An iterator for a given object's inventory."""
    current_item = obj.Inventory
    while current_item != None:
        next_item = current_item.Below
        yield current_item
        current_item = next_item

def deposit_box_close():
    """Find the total value of items in the deposit box and deposit."""
    total_value = 0
    for obj in get_inventory(whoami):
        if obj.Name != 'event_close':
            total_value += obj.Value * obj.Quantity
            obj.Remove()
    with CFBank.open() as bank:
        bank.deposit(activator.Name, total_value)
        whoami.Say("%s credited to your account." \
                % Crossfire.CostStringFromValue(total_value))

deposit_box_close()
