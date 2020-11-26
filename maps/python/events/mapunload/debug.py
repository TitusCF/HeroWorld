import Crossfire

import utils


map_ = Crossfire.WhoAmI()
utils.log_debug('%s %s' % ('mapunload', map_.Path))
