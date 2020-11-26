import Crossfire

import utils


map_ = Crossfire.WhoAmI()
utils.log_debug('%s %s' % ('mapreset', map_.Path))
