import Crossfire

world_prefix = '/world/world_'
world_len = len( world_prefix ) + len( 'xxx_xxx' )
world_sep = '_'
world_map_size = 50

Crossfire.SetReturnValue( 1 )

player = Crossfire.WhoIsActivator()
gps = Crossfire.WhoAmI()
map = player.Map

if ( map == 0 ):
	player.Write( 'You\'re lost in a vacuum!')
else:
	path = map.Path
	if ( path.find( world_prefix ) != 0 ) or ( len( path ) != world_len ):
		player.Write( 'You can\'t position yourself here.' )
	else:
		marked = player.MarkedItem

		if ( marked != gps ) and ( gps.Food == 0 ):
			player.Write( 'You must fix the origin of the positioning system first!' )
		else:
			coord = path.split( world_sep )
			if ( len( coord ) != 3 ):
				player.Write( 'Strange place, you can\'t position yourself...' )
			else:
				map_x = int( coord[ 1 ] ) - 99
				map_y = int( coord[ 2 ] ) - 99
				x = map_x * world_map_size + player.X
				y = map_y * world_map_size + player.Y

				if ( marked == gps ):
					gps.HP=x
					gps.SP=y
					gps.Food=1
					player.Write( 'You reset the origin of the system.' )
				else:
					x = x - gps.HP
					y = y - gps.SP
					player.Write( 'You are at %s:%s.'%( x, y ))
