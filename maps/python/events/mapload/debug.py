import Crossfire

import utils


map_ = Crossfire.WhoAmI()
utils.log_debug('%s %s' % ('mapload', map_.Path))
