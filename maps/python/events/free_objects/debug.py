import Crossfire

import utils


map_ = Crossfire.WhoAmI()
utils.log_debug('%s %s' % ('free_objects', map_.Path))
