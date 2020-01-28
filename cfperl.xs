/*
 * This file is part of Deliantra, the Roguelike Realtime MMORPG.
 * 
 * Copyright (©) 2006,2007,2008,2009,2010 Marc Alexander Lehmann / Robin Redeker / the Deliantra team
 * 
 * Deliantra is free software: you can redistribute it and/or modify it under
 * the terms of the Affero GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the Affero GNU General Public License
 * and the GNU General Public License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 * 
 * The authors can be reached via e-mail to <support@deliantra.net>
 */

#include "autoconf.h"

#if HAVE_EXECINFO_H
# include <execinfo.h>
#endif

#include <cstdarg>
#include <typeinfo>

#include "global.h"
#include "../random_maps/random_map.h"
#include "evthread.h"
#include "sproto.h"

#include <unistd.h>
#if _POSIX_MEMLOCK
# include <sys/mman.h>
#endif

#if HAVE_MALLOC_H
# include <malloc.h>
#endif

#if !__GLIBC__
# define malloc_trim(pad) -1
#endif

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "CoroAPI.h"
#include "perlxsi.c"

typedef object_thawer  &object_thawer_ref;
typedef object_freezer &object_freezer_ref;

typedef std::string std__string;

static PerlInterpreter *perl;

tstamp NOW, runtime;

static int tick_inhibit;
static int tick_pending;

global gbl_ev;
static AV *cb_global, *cb_attachable, *cb_object, *cb_player, *cb_client, *cb_type, *cb_map;
static SV *sv_runtime, *sv_tick_start, *sv_next_tick, *sv_now;
static AV *av_reflect;

bitset<NUM_EVENT_TYPES> ev_want_event;
bitset<NUM_TYPES>       ev_want_type;

static HV
   *stash_cf,
   *stash_cf_object_wrap,
   *stash_cf_object_player_wrap,
   *stash_cf_player_wrap,
   *stash_cf_map_wrap,
   *stash_cf_mapspace_wrap,
   *stash_cf_client_wrap,
   *stash_cf_arch_wrap,
   *stash_cf_party_wrap,
   *stash_cf_region_wrap,
   *stash_cf_living_wrap,
   *stash_ext_map_world;

static SV
   *cv_cf_do_invoke,
   *cv_cf__can_merge,
   *cv_cf_client_send_msg,
   *cv_cf_tick,
   *cv_cf_match_match;

#ifndef newSVpv_utf8
static SV *
newSVpv_utf8 (const char *s)
{
  if (!s)
    return newSV (0);

  SV *sv = newSVpv (s, 0);
  SvUTF8_on (sv);
  return sv;
}
#endif

#ifndef newSVpvn_utf8
static SV *
newSVpvn_utf8 (const char *s, STRLEN l, int utf8)
{
  if (!s)
    return newSV (0);

  SV *sv = newSVpvn (s, l);

  if (utf8)
    SvUTF8_on (sv);

  return sv;
}
#endif

static noinline utf8_string
cfSvPVutf8_nolen (SV *sv)
{
  SvGETMAGIC (sv);

  if (SvPOK (sv))
    {
      if (!SvUTF8 (sv))
        sv_utf8_upgrade_nomg (sv);

      return SvPVX (sv);
    }

  return SvPV_nolen (sv);
}

// helper cast function, returns super class * or 0
template<class super>
static super *
is_a (attachable *at)
{
  //return dynamic_cast<super *>(at); // slower, safer
  if (typeid (*at) == typeid (super))
    return static_cast<super *>(at);
  else
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

unordered_vector<attachable *> attachable::mortals;

attachable::~attachable ()
{
  assert (!self);
  assert (!cb);
}

int
attachable::refcnt_cnt () const
{
  return refcnt + (self ? SvREFCNT (self) - 1 : 0);
}

void
attachable::sever_self ()
{
  if (HV *self = this->self)
    {
      // keep a refcount because sv_unmagic might call attachable_free,
      // which might clear self, causing sv_unmagic to crash on a now
      // invalid object.
      SvREFCNT_inc (self);
      hv_clear (self);
      sv_unmagic ((SV *)self, PERL_MAGIC_ext);
      SvREFCNT_dec (self);

      // self *must* be null now because that's sv_unmagic's job.
      assert (!this->self);
    }
}

void
attachable::optimise ()
{
  if (self
      && SvREFCNT (self) == 1
      && !HvTOTALKEYS (self))
    sever_self ();
}

// check wether the object really is dead
void
attachable::do_check ()
{
  if (refcnt_cnt () > 0)
    return;

  destroy ();
}

void
attachable::do_destroy ()
{
  INVOKE_ATTACHABLE (DESTROY, this);

  if (cb)
    {
      SvREFCNT_dec (cb);
      cb = 0;
    }

  mortals.push_back (this);
}

void
attachable::destroy ()
{
  if (destroyed ())
    return;

  attachable_flags |= F_DESTROYED;
  do_destroy ();
  sever_self ();
}

void
attachable::do_delete ()
{
  delete this;
}

void
attachable::check_mortals ()
{
  static int i = 0;

  for (;;)
    {
      if (i >= mortals.size ())
        {
          i = 0;

          if (mortals.size () >= 512)
            {
              static int last_mortalcount;
              if (mortals.size () != last_mortalcount)
                {
                  last_mortalcount = mortals.size ();
                  LOG (llevInfo, "%d mortals.\n", (int)mortals.size ());

                  if (0)
                    {
                      for (int j = 0; j < mortals.size (); ++j)//D
                        fprintf (stderr, "%d:%s %p ", j, &((object *)mortals[j])->name, mortals[j]);//D

                      fprintf (stderr, "\n");//D
                    }
                }
            }

          break;
        }

      attachable *obj = mortals [i];

#if 0
      if (obj->self)//D make this an assert later
        {
          LOG (llevError, "check_mortals: object '%s' still has self\n", typeid (obj).name ());
          obj->sever_self ();
        }
#endif

      if (obj->refcnt)
        {
          ++i; // further delay freeing

          if (!(i & 0x3ff))
            break;
        }
      else
        {
          mortals.erase (i);
          obj->sever_self ();
          obj->do_delete ();
        }
    }
}

void
attachable::set_key (const char *key, const char *value, bool is_utf8)
{
  if (!self)
    self = newHV ();

  if (value)
    hv_store (self, key, strlen (key), is_utf8 ? newSVpv_utf8 (value) : newSVpv (value, 0), 0);
  else
    hv_delete (self, key, strlen (key), G_DISCARD);
}

attachable &
attachable::operator =(const attachable &src)
{
  //if (self || cb)
    //INVOKE_OBJECT (CLONE, this, ARG_OBJECT (dst));

  attach = src.attach;
  return *this;
}

#if 0
template<typename T>
static bool
find_backref (void *ptr, T *obj)
{
  char *s = (char *)obj;
  while (s < (char *)obj + sizeof (T))
    {
      if (ptr == *(void **)s)
        return true;

      s += sizeof (void *); // assume natural alignment
    }

  return false;
}

// for debugging, find "live" objects containing this ptr
static void
find_backref (void *ptr)
{
  for_all_objects (op)
    if (find_backref (ptr, op))
      fprintf (stderr, "O %p %d:'%s'\n", op, op->count, &op->name);

  for_all_players (pl)
    if (find_backref (ptr, pl))
      fprintf (stderr, "P %p\n", pl);
  
  for_all_clients (ns)
    if (find_backref (ptr, ns))
      fprintf (stderr, "C %p\n", ns);
}
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SV *
newSVptr (void *ptr, HV *stash, HV *hv)
{
  SV *sv;

  if (!ptr)
    return newSV (0);

  sv_magicext ((SV *)hv, 0, PERL_MAGIC_ext, 0, (char *)ptr, 0);
  return sv_bless (newRV_noinc ((SV *)hv), stash);
}

static SV *
newSVptr (void *ptr, HV *stash)
{
  return newSVptr (ptr, stash, newHV ());
}

static int
attachable_free (pTHX_ SV *sv, MAGIC *mg)
{
  attachable *at = (attachable *)mg->mg_ptr;

  //TODO: check if transaction behaviour is really required here
  if (SV *self = (SV *)at->self)
    {
      at->self = 0;
      SvREFCNT_dec (self);
    }

  // next line makes sense, but most objects still have refcnt 0 by default
  //at->refcnt_chk ();
  return 0;
}

MGVTBL attachable::vtbl = {0, 0, 0, 0, attachable_free};

static SV *
newSVattachable (attachable *obj, HV *stash)
{
  if (!obj)
    return newSV (0);

  if (!obj->self)
    obj->self = newHV ();

  if (!SvOBJECT (obj->self))
    {
      sv_magicext ((SV *)obj->self, 0, PERL_MAGIC_ext, &attachable::vtbl, (char *)obj, 0);

      // now bless the object _once_
      //TODO: create a class registry with c++ type<=>perl name<=>stash and use it here and elsewhere
      return sv_bless (newRV_inc ((SV *)obj->self), stash);
    }
  else
    {
      SV *sv = newRV_inc ((SV *)obj->self);

      if (Gv_AMG (stash)) // handle overload correctly, as the perl core does not
        SvAMAGIC_on (sv);

      return sv;
    }
}

#if 0 // unused
static void
clearSVptr (SV *sv)
{
  if (SvROK (sv))
    sv = SvRV (sv);

  hv_clear ((HV *)sv);
  sv_unmagic (sv, PERL_MAGIC_ext);
}
#endif

static long
SvPTR_nc (SV *sv)
{
  sv = SvRV (sv);

  // very important shortcut
  if (expect_true (SvMAGIC (sv) && SvMAGIC (sv)->mg_type == PERL_MAGIC_ext))
    return (long)SvMAGIC (sv)->mg_ptr;

  if (MAGIC *mg = mg_find (sv, PERL_MAGIC_ext))
    return (long)mg->mg_ptr;

  croak ("perl code used object, but C object is already destroyed, caught");
}

static long
SvPTR (SV *sv, const char *klass)
{
  if (!sv_derived_from (sv, klass))
    croak ("object of type %s expected", klass);

  return SvPTR_nc (sv);
}

static long noinline
SvPTR_ornull (SV *sv, const char *klass)
{
  if (expect_false (!SvOK (sv))) return 0;

  return SvPTR (sv, klass);
}

static long noinline
SvPTR_ornull_client (SV *sv)
{
  if (expect_false (!SvOK (sv))) return 0;

  if (!SvROK (sv)
      || (SvSTASH (SvRV (sv)) != stash_cf_client_wrap
          && !sv_derived_from (sv, "cf::client")))
    croak ("object of type cf::client expected");

  return SvPTR_nc (sv);
}

static long noinline
SvPTR_ornull_object (SV *sv)
{
  if (expect_false (!SvOK (sv))) return 0;

  if (!SvROK (sv)
      || (SvSTASH (SvRV (sv)) != stash_cf_object_wrap
          && SvSTASH (SvRV (sv)) != stash_cf_object_player_wrap
          && SvSTASH (SvRV (sv)) != stash_cf_arch_wrap
          && !sv_derived_from (sv, "cf::object")))
    croak ("object of type cf::object expected");

  return SvPTR_nc (sv);
}

static long noinline
SvPTR_ornull_maptile (SV *sv)
{
  if (expect_false (!SvOK (sv))) return 0;

  if (!SvROK (sv)
      || (SvSTASH (SvRV (sv)) != stash_cf_map_wrap
          && SvSTASH (SvRV (sv)) != stash_ext_map_world
          && !sv_derived_from (sv, "cf::map")))
    croak ("object of type cf::map expected");

  return SvPTR_nc (sv);
}

static long noinline
SvPTR_ornull_player (SV *sv)
{
  if (expect_false (!SvOK (sv))) return 0;

  if (!SvROK (sv)
      || (SvSTASH (SvRV (sv)) != stash_cf_player_wrap
          && !sv_derived_from (sv, "cf::player")))
    croak ("object of type cf::player expected");

  return SvPTR_nc (sv);
}

static inline SV *to_sv (const shstr &  v) { return newSVpvn_utf8 ((const char *)v, v.length (), 1); }
static inline SV *to_sv (const char *   v) { return v ? newSVpv (v, 0) : newSV (0); }
static inline SV *to_sv (bool           v) { return newSViv (v); }
static inline SV *to_sv (  signed char  v) { return newSViv (v); }
static inline SV *to_sv (unsigned char  v) { return newSViv (v); }
static inline SV *to_sv (  signed short v) { return newSViv (v); }
static inline SV *to_sv (unsigned short v) { return newSVuv (v); }
static inline SV *to_sv (  signed int   v) { return newSViv (v); }
static inline SV *to_sv (unsigned int   v) { return newSVuv (v); }
static inline SV *to_sv (  signed long  v) { return newSViv (v); }
static inline SV *to_sv (unsigned long  v) { return newSVuv (v); }
static inline SV *to_sv (  signed long long v) { return newSVval64 (v); }
static inline SV *to_sv (unsigned long long v) { return newSVval64 (v); }
static inline SV *to_sv (float          v) { return newSVnv (v); }
static inline SV *to_sv (double         v) { return newSVnv (v); }
static inline SV *to_sv (client *       v) { return newSVattachable (v, stash_cf_client_wrap); }
static inline SV *to_sv (player *       v) { return newSVattachable (v, stash_cf_player_wrap); }
static inline SV *to_sv (object *       v) { return newSVattachable (v, v && v->type == PLAYER ? stash_cf_object_player_wrap : stash_cf_object_wrap); }
static inline SV *to_sv (maptile *      v) { return newSVattachable (v, stash_cf_map_wrap); }
static inline SV *to_sv (archetype *    v) { return newSVattachable (v, stash_cf_arch_wrap); }
static inline SV *to_sv (region *       v) { return newSVattachable (v, stash_cf_region_wrap); }
static inline SV *to_sv (partylist *    v) { return newSVptr (v, stash_cf_party_wrap); }
static inline SV *to_sv (living *       v) { return newSVptr (v, stash_cf_living_wrap); }
static inline SV *to_sv (mapspace *     v) { return newSVptr (v, stash_cf_mapspace_wrap); }

static inline SV *to_sv (object &       v) { return to_sv (&v); }
static inline SV *to_sv (living &       v) { return to_sv (&v); }

static inline SV *to_sv (const std::string & v) { return newSVpvn (v.data (), v.size ()); }
static inline SV *to_sv (const treasurelist *v) { return to_sv (v->name); }

static inline SV *to_sv (UUID           v) { return newSVpv (v.c_str (), 0); }

static inline SV *to_sv (dynbuf *       v)
{
  SV *sv = newSV (0);

  sv_upgrade (sv, SVt_PV);
  SvGROW (sv, v->size () + 1);
  SvPOK_only (sv);
  v->linearise (SvPVX (sv));
  SvCUR_set (sv, v->size ());
  *SvEND (sv) = 0;

  return sv;
}

static inline SV *to_sv (dynbuf_text *  v)
{
  SV *sv = to_sv (static_cast<dynbuf *> (v));
  SvUTF8_on (sv);
  return sv;
}

static inline void sv_to (SV *sv, shstr              &v) { v = SvOK (sv) ? cfSvPVutf8_nolen (sv) : 0; }
static inline void sv_to (SV *sv, char *             &v) { free (v); v = SvOK (sv) ? strdup (SvPV_nolen (sv)) : 0; }
static inline void sv_to (SV *sv, bool               &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv,   signed char      &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv, unsigned char      &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv,   signed short     &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv, unsigned short     &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv,   signed int       &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv, unsigned int       &v) { v = SvUV (sv); }
static inline void sv_to (SV *sv,   signed long      &v) { v = SvIV (sv); }
static inline void sv_to (SV *sv, unsigned long      &v) { v = SvUV (sv); }
static inline void sv_to (SV *sv,   signed long long &v) { v = (  signed long long)SvVAL64 (sv); }
static inline void sv_to (SV *sv, unsigned long long &v) { v = (unsigned long long)SvVAL64 (sv); }
static inline void sv_to (SV *sv, float              &v) { v = SvNV (sv); }
static inline void sv_to (SV *sv, double             &v) { v = SvNV (sv); }
static inline void sv_to (SV *sv, client *           &v) { v = (client *)   (attachable *)SvPTR_ornull_client  (sv); }
static inline void sv_to (SV *sv, player *           &v) { v = (player *)   (attachable *)SvPTR_ornull_player  (sv); }
static inline void sv_to (SV *sv, object *           &v) { v = (object *)   (attachable *)SvPTR_ornull_object  (sv); }
static inline void sv_to (SV *sv, maptile *          &v) { v = (maptile *)  (attachable *)SvPTR_ornull_maptile (sv); }
static inline void sv_to (SV *sv, archetype *        &v) { v = (archetype *)(attachable *)SvPTR_ornull (sv, "cf::arch"); }
static inline void sv_to (SV *sv, region *           &v) { v = (region *)   (attachable *)SvPTR_ornull (sv, "cf::region"); }
static inline void sv_to (SV *sv, attachable *       &v) { v =              (attachable *)SvPTR_ornull (sv, "cf::attachable"); }
static inline void sv_to (SV *sv, partylist *        &v) { v = (partylist *)              SvPTR_ornull (sv, "cf::party"); }
static inline void sv_to (SV *sv, living *           &v) { v = (living *)                 SvPTR_ornull (sv, "cf::living"); }
static inline void sv_to (SV *sv, mapspace *         &v) { v = (mapspace *)               SvPTR_ornull (sv, "cf::mapspace"); }
static inline void sv_to (SV *sv, object_freezer *   &v) { v = (object_freezer *)         SvPTR_ornull (sv, "cf::object::freezer"); }
static inline void sv_to (SV *sv, object_thawer *    &v) { v = (object_thawer  *)         SvPTR_ornull (sv, "cf::object::thawer" ); }

//static inline void sv_to (SV *sv, faceinfo *         &v) { v = &faces [face_find (SvPV_nolen (sv), 0)]; }
static inline void sv_to (SV *sv, treasurelist *     &v) { v = treasurelist::find (SvPV_nolen (sv)); }

template<class T>
static inline void sv_to (SV *sv, refptr<T>          &v) { T *tmp; sv_to (sv, tmp); v = tmp; }

template<int N>
static inline void sv_to (SV *sv, char      (&v)[N]) { assign (v, SvPV_nolen (sv)); }

static inline void sv_to (SV *sv, bowtype_t          &v) { v = (bowtype_t)   SvIV (sv); }
static inline void sv_to (SV *sv, petmode_t          &v) { v = (petmode_t)   SvIV (sv); }
static inline void sv_to (SV *sv, usekeytype         &v) { v = (usekeytype)  SvIV (sv); }
static inline void sv_to (SV *sv, unapplymode        &v) { v = (unapplymode) SvIV (sv); }

static inline void sv_to (SV *sv, std::string        &v)
{
  STRLEN len;
  char *data = SvPVbyte (sv, len);
  v.assign (data, len);
}

static inline void sv_to (SV *sv, UUID               &v)
{
  if (!v.parse (SvPV_nolen (sv)))
    croak ("unparsable uuid: %s", SvPV_nolen (sv));
}

static inline void sv_to (SV *sv, object::flags_t::reference v) { v = SvTRUE (sv); }

static SV *
newSVdt_va (va_list &ap, data_type type)
{
  SV *sv;

  switch (type)
    {
      case DT_INT:
        sv = newSViv (va_arg (ap, int));
        break;

      case DT_INT64:
        sv = newSVval64 ((val64)va_arg (ap, sint64));
        break;

      case DT_DOUBLE:
        sv = newSVnv (va_arg (ap, double));
        break;

      case DT_STRING:
        {
          char *str = (char *)va_arg (ap, const char *);
          sv = str ? newSVpv (str, 0) : newSV (0);
        }
        break;

      case DT_DATA:
        {
          char *str = (char *)va_arg (ap, const void *);
          int len = va_arg (ap, int);
          sv = str ? newSVpv (str, len) : newSV (0);
        }
        break;

      case DT_OBJECT:
        sv = to_sv (va_arg (ap, object *));
        break;

      case DT_MAP:
        // va_arg (object *) when void * is passed is an XSI extension
        sv = to_sv (va_arg (ap, maptile *));
        break;

      case DT_CLIENT:
        sv = to_sv (va_arg (ap, client *));
        break;

      case DT_PLAYER:
        sv = to_sv (va_arg (ap, player *));
        break;

      case DT_ARCH:
        sv = to_sv (va_arg (ap, archetype *));
        break;

      case DT_PARTY:
        sv = to_sv (va_arg (ap, partylist *));
        break;

      case DT_REGION:
        sv = to_sv (va_arg (ap, region *));
        break;

      default:
        assert (("unhandled type in newSVdt_va", 0));
    }

  return sv;
}

static SV *
newSVdt (data_type type, ...)
{
  va_list ap;

  va_start (ap, type);
  SV *sv = newSVdt_va (ap, type);
  va_end (ap);

  return sv;
}

// typemap support, mostly to avoid excessive inlining
template<class type>
static void noinline
cf_obj_to (SV *arg, type &var)
{
  sv_to (arg, var);
  if (expect_false (!var))
    croak ("must not pass invalid/null cf_obj here");
}

template<class object>
static void noinline
cf_obj_ornull_to (SV *arg, object *&var)
{
  if (SvOK (arg))
    {
      sv_to (arg, var);
      if (expect_false (!var))
        croak ("unable to convert perl object to C++ object");
    }
  else
    var = 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

static SV *
registry (attachable *ext)
{
  if (!ext->cb)
    ext->cb = newAV ();

  return newRV_inc ((SV *)ext->cb);
}

/////////////////////////////////////////////////////////////////////////////

void
cfperl_init ()
{
  extern char **environ;

  PERL_SYS_INIT3 (&settings.argc, &settings.argv, &environ);
  perl = perl_alloc ();
  perl_construct (perl);

  PL_exit_flags |= PERL_EXIT_DESTRUCT_END;

  const char *argv[] = {
    settings.argv [0],
    "-e0"
  };

  if (perl_parse (perl, xs_init, 2, (char **)argv, environ)
      || perl_run (perl))
    {
      printf ("unable to initialize perl-interpreter, aborting.\n");
      exit (EXIT_FAILURE);
    }

  eval_pv (
    "#line 1 'cfperl init'\n"
    "use EV ();\n"      // required by bootstrap
    "use Coro ();\n"    // required by bootstrap
    "cf->bootstrap;\n"  // required for cf::datadir
    "unshift @INC, cf::datadir ();\n" // required for 'require' :)
    "require cf;\n",
    0
  );

  if (SvTRUE (ERRSV))
    {
      printf ("unable to bootstrap perl, aborting:\n%s", SvPV_nolen (ERRSV));
      exit (EXIT_FAILURE);
    }
}

void
cfperl_main ()
{
  dSP;

  PUSHMARK (SP);
  PUTBACK;
  call_pv ("cf::main", G_DISCARD | G_VOID);
}

void
attachable::instantiate ()
{
  if (attach)
    {
      INVOKE_ATTACHABLE (INSTANTIATE, this, ARG_STRING (attach));
      attach = 0;
    }
}

void
attachable::reattach ()
{
  optimise ();
  //TODO: check for _attachment's, very important for restarts
  INVOKE_ATTACHABLE (REATTACH, this);
}

static event_klass klass_of[NUM_EVENT_TYPES] = {
# define def(type,name) KLASS_ ## type,
#  include "eventinc.h"
# undef def
};

#define KLASS_OF(event) (((unsigned int)event) < NUM_EVENT_TYPES ? klass_of [event] : KLASS_NONE)

static void noinline
gather_callbacks (AV *&callbacks, AV *registry, event_type event)
{
  // event must be in array
  if (event >= 0 && event <= AvFILLp (registry))
    {
      SV *cbs_ = AvARRAY (registry)[event];

      // element must be list of callback entries
      if (cbs_ && SvROK (cbs_) && SvTYPE (SvRV (cbs_)) == SVt_PVAV)
        {
          AV *cbs = (AV *)SvRV (cbs_);

          // no callback entries, no callbacks to call
          if (AvFILLp (cbs) >= 0)
            {
              if (!callbacks)
                {
                  callbacks = newAV ();
                  av_extend (callbacks, 16);
                }
              
              // never use SvREFCNT_inc to copy values, but its ok here :)
              for (int i = 0; i <= AvFILLp (cbs); ++i)
                av_push (callbacks, SvREFCNT_inc (AvARRAY (cbs)[i]));
            }
        }
    }
}

void
attachable::gather_callbacks (AV *&callbacks, event_type event) const
{
  ::gather_callbacks (callbacks, cb_attachable, event);

  if (cb)
    ::gather_callbacks (callbacks, cb, event);
}

void
global::gather_callbacks (AV *&callbacks, event_type event) const
{
  ::gather_callbacks (callbacks, cb_global, event);
}

void
object::gather_callbacks (AV *&callbacks, event_type event) const
{
  if (subtype && type + subtype * NUM_TYPES <= AvFILLp (cb_type))
    {
      SV *registry = AvARRAY (cb_type)[type + subtype * NUM_TYPES];

      if (registry && SvROK (registry) && SvTYPE (SvRV (registry)) == SVt_PVAV)
        ::gather_callbacks (callbacks, (AV *)SvRV (registry), event);
    }

  if (type <= AvFILLp (cb_type))
    {
      SV *registry = AvARRAY (cb_type)[type];

      if (registry && SvROK (registry) && SvTYPE (SvRV (registry)) == SVt_PVAV)
        ::gather_callbacks (callbacks, (AV *)SvRV (registry), event);
    }

  attachable::gather_callbacks (callbacks, event);
  ::gather_callbacks (callbacks, cb_object, event);
}

void
archetype::gather_callbacks (AV *&callbacks, event_type event) const
{
  attachable::gather_callbacks (callbacks, event);
  //TODO//::gather_callbacks (callbacks, cb_archetype, event);
}

void
client::gather_callbacks (AV *&callbacks, event_type event) const
{
  attachable::gather_callbacks (callbacks, event);
  ::gather_callbacks (callbacks, cb_client, event);
}

void
player::gather_callbacks (AV *&callbacks, event_type event) const
{
  attachable::gather_callbacks (callbacks, event);
  ::gather_callbacks (callbacks, cb_player, event);
}

void
maptile::gather_callbacks (AV *&callbacks, event_type event) const
{
  attachable::gather_callbacks (callbacks, event);
  ::gather_callbacks (callbacks, cb_map, event);
}

static void noinline
_recalc_want (bitset<NUM_EVENT_TYPES> &set, AV *registry)
{
  for (int event = 0; event <= AvFILLp (registry); ++event)
    {
      SV *cbs_ = AvARRAY (registry)[event];

      // element must be list of callback entries
      if (cbs_ && SvROK (cbs_) && SvTYPE (SvRV (cbs_)) == SVt_PVAV)
        {
          AV *cbs = (AV *)SvRV (cbs_);

          // no callback entries, no callbacks to call
          if (AvFILLp (cbs) >= 0)
            set.set (event);
        }
    }
}

// very slow and inefficient way to recalculate the global want bitsets
static void
_recalc_want ()
{
  ev_want_event.reset ();

  _recalc_want (ev_want_event, cb_global);
  _recalc_want (ev_want_event, cb_attachable);
  _recalc_want (ev_want_event, cb_object);
  _recalc_want (ev_want_event, cb_client);
  _recalc_want (ev_want_event, cb_player);
  _recalc_want (ev_want_event, cb_map);

  ev_want_type.reset ();

  for (int type = 0; type <= AvFILLp (cb_type); ++type)
    {
      SV *cbs_ = AvARRAY (cb_type)[type];

      // element must be list of callback entries
      if (cbs_ && SvROK (cbs_) && SvTYPE (SvRV (cbs_)) == SVt_PVAV)
        {
          AV *cbs = (AV *)SvRV (cbs_);

          // no callback entries, no callbacks to call
          if (AvFILLp (cbs) >= 0)
            ev_want_type.set (type % NUM_TYPES);
        }
    }
}

bool
attachable::invoke (event_type event, ...)
{
  data_type dt;

  // callback call ordering should be:
  // 1. per-object callback
  // 2. per-class object
  // 3. per-type callback
  // 4. global callbacks

  AV *callbacks = 0;
  gather_callbacks (callbacks, event);

  // short-circuit processing if no callbacks found/defined
  if (!callbacks)
    return 0;

  va_list ap;
  va_start (ap, event);

  CALL_BEGIN (3);
  CALL_ARG_SV (newSViv (event)); // only used for debugging nowadays
  CALL_ARG_SV (newRV_noinc ((SV *)callbacks));

  //TODO: unhack
  if (object *op = is_a<object>(this))       CALL_ARG_SV (newSVdt (DT_OBJECT, op));
  else if (player *pl = is_a<player>(this))  CALL_ARG_SV (newSVdt (DT_PLAYER, pl));
  else if (client *ns = is_a<client>(this))  CALL_ARG_SV (newSVdt (DT_CLIENT, ns));
  else if (maptile *m = is_a<maptile>(this)) CALL_ARG_SV (newSVdt (DT_MAP, m));
  else if (global *gl = is_a<global>(this))  /*nop*/;
  else
    abort (); //TODO

  for (;;)
    {
      dt = (data_type) va_arg (ap, int);

      if (dt == DT_END)
        break;
      else if (dt == DT_AV)
        {
          AV *av = va_arg (ap, AV *);

          for (int i = 0; i <= av_len (av); ++i)
            XPUSHs (*av_fetch (av, i, 1));
        }
      else
        XPUSHs (sv_2mortal (newSVdt_va (ap, dt)));
    }

  va_end (ap);

  CALL_CALL (cv_cf_do_invoke, G_SCALAR);
  count = count > 0 ? POPi : 0;

  CALL_END;

  return count;
}

static SV *
cfperl_result (int idx)
{
  AV *av = get_av ("cf::INVOKE_RESULTS", 0);
  if (!av)
    return &PL_sv_undef;

  SV **sv = av_fetch (av, idx, 0);
  if (!sv)
    return &PL_sv_undef;

  return *sv;
}

int
cfperl_result_INT (int idx)
{
  return SvIV (cfperl_result (idx));
}

double
cfperl_result_DOUBLE (int idx)
{
  return SvNV (cfperl_result (idx));
}

/////////////////////////////////////////////////////////////////////////////
// various c++ => perl glue functions

void
cfperl_tick ()
{
  tick_pending = 1;

  if (tick_inhibit)
    return;

  tick_pending = 0;

  dSP;

  PUSHMARK (SP);
  PUTBACK;
  call_pvsv (cv_cf_tick, G_DISCARD | G_VOID);

  SvNV_set (sv_next_tick, get_next_tick ()); SvNOK_only (sv_next_tick);
}

void
cfperl_emergency_save ()
{
  CALL_BEGIN (0);
  CALL_CALL ("cf::emergency_save", G_VOID);
  CALL_END;
}

void
cfperl_cleanup (int make_core)
{
  CALL_BEGIN (1);
  CALL_ARG (make_core);
  CALL_CALL ("cf::post_cleanup", G_VOID);
  CALL_END;
}

void
cfperl_make_book (object *book, int level)
{
  CALL_BEGIN (2);
  CALL_ARG (book);
  CALL_ARG (level);
  CALL_CALL ("ext::books::make_book", G_VOID);
  CALL_END;
}

void
cfperl_send_msg (client *ns, int color, const_utf8_string type, const_utf8_string msg)
{
  CALL_BEGIN (4);
  CALL_ARG (ns);
  CALL_ARG (type);
  CALL_ARG_SV (newSVpv_utf8 (msg));
  CALL_ARG (color);
  CALL_CALL (cv_cf_client_send_msg, G_VOID);
  CALL_END;
}

int
cfperl_can_merge (object *ob1, object *ob2)
{
  int can;

  CALL_BEGIN (2);
  CALL_ARG (ob1);
  CALL_ARG (ob2);
  CALL_CALL (cv_cf__can_merge, G_SCALAR);
  can = count && SvTRUE (TOPs);
  CALL_END;

  return can;
}

void
cfperl_mapscript_activate (object *ob, int state, object *activator, object *originator)
{
  CALL_BEGIN (4);
  CALL_ARG (ob);
  CALL_ARG (state);
  CALL_ARG (activator);
  CALL_ARG (originator);
  CALL_CALL ("cf::mapscript::activate", G_VOID);
  CALL_END;
}

player *
player::find (const_utf8_string name)
{
  CALL_BEGIN (1);
  CALL_ARG (name);
  CALL_CALL ("cf::player::find", G_SCALAR);

  player *retval = 0;
  if (count) sv_to (POPs, retval);

  CALL_END;

  return retval;
}

maptile *
find_style (const_utf8_string dirname, const_utf8_string stylename, int difficulty, bool recurse)
{
  CALL_BEGIN (4);
  CALL_ARG (dirname);
  CALL_ARG (stylename);
  CALL_ARG (difficulty);
  CALL_ARG (recurse);
  CALL_CALL ("ext::map_random::find_style", G_SCALAR);

  maptile *retval = 0;
  if (count) sv_to (POPs, retval);

  CALL_END;

  return retval;
}

maptile *
maptile::find_sync (const_utf8_string path, maptile *origin)
{
  CALL_BEGIN (2);
  CALL_ARG (path);
  CALL_ARG (origin);
  CALL_CALL ("cf::map::find_sync", G_SCALAR);

  maptile *retval = 0;
  if (count) sv_to (POPs, retval);

  CALL_END;

  return retval;
}

maptile *
maptile::find_async (const_utf8_string path, maptile *origin, bool load)
{
  CALL_BEGIN (3);
  CALL_ARG (path);
  CALL_ARG (origin);
  CALL_ARG (load);
  CALL_CALL ("cf::map::find_async", G_SCALAR);

  maptile *retval = 0;
  if (count) sv_to (POPs, retval);

  CALL_END;

  return retval;
}

void
maptile::do_load_sync ()
{
  CALL_BEGIN (1);
  CALL_ARG (this);
  CALL_CALL ("cf::map::do_load_sync", G_SCALAR);
  CALL_END;
}

void
object::enter_exit (object *exit)
{
  if (type != PLAYER)
    return;

  CALL_BEGIN (2);
  CALL_ARG (this);
  CALL_ARG (exit);
  CALL_CALL ("cf::object::player::enter_exit", G_VOID);
  CALL_END;
}

void
object::player_goto (const_utf8_string path, int x, int y)
{
  if (type != PLAYER)
    return;

  CALL_BEGIN (4);
  CALL_ARG (this);
  CALL_ARG (path);
  CALL_ARG (x);
  CALL_ARG (y);
  CALL_CALL ("cf::object::player::goto", G_VOID);
  CALL_END;
}

const_utf8_string 
object::ref () const
{
  if (type == PLAYER)
    return format ("player/<1.%llx>/%s", (unsigned long long)uuid.seq, &name);
  else
    // TODO: should be able to save references within the same map, at least
    return 0;
}

object *
object::deref (const_utf8_string ref)
{
  object *retval = 0;

  if (ref)
    {
      CALL_BEGIN (1);
      CALL_ARG (ref);
      CALL_CALL ("cf::object::deref", G_SCALAR);

      if (count)
        sv_to (POPs, retval);

      CALL_END;
    }

  return retval;
}

void
log_backtrace (const_utf8_string msg)
{
#if HAVE_BACKTRACE
  void *addr [20];
  int size = backtrace (addr, 20);

  CALL_BEGIN (size);
  CALL_ARG (msg);
  for (int i = 0; i < size; ++i)
    CALL_ARG ((IV)addr [i]);
  CALL_CALL ("cf::_log_backtrace", G_VOID);
  CALL_END;
#endif
}

bool
is_match_expr (const_utf8_string expr)
{
  return !strncmp (expr, "match ", sizeof ("match ") - 1);
}

bool
match (const_utf8_string expr, object *ob, object *self, object *source, object *originator)
{
  if (!strncmp (expr, "match ", sizeof ("match ") - 1))
    expr += sizeof ("match ") - 1;

  CALL_BEGIN (5);
  CALL_ARG (expr);
  CALL_ARG (ob);
  CALL_ARG (self);
  CALL_ARG (source);
  CALL_ARG (originator);
  CALL_CALL (cv_cf_match_match, G_SCALAR);

  bool matched = count && SvTRUE (TOPs);

  CALL_END;

  return matched;
}

object *
match_one (const_utf8_string expr, object *ob, object *self, object *source, object *originator)
{
  if (!strncmp (expr, "match ", sizeof ("match ") - 1))
    expr += sizeof ("match ") - 1;

  CALL_BEGIN (5);
  CALL_ARG (expr);
  CALL_ARG (ob);
  CALL_ARG (self);
  CALL_ARG (source);
  CALL_ARG (originator);
  CALL_CALL (cv_cf_match_match, G_ARRAY);
  
  object *one = 0;

  if (count)
    sv_to (TOPs, one);

  CALL_END;

  return one;
}

/////////////////////////////////////////////////////////////////////////////

struct EVAPI   *evapi::GEVAPI;
struct CoroAPI *coroapi::GCoroAPI;

void
coroapi::do_cede_to_tick ()
{
  cede_pending = 0;
  cede ();
}

void
coroapi::wait_for_tick ()
{
  CALL_BEGIN (0);
  CALL_CALL ("cf::wait_for_tick", G_DISCARD);
  CALL_END;
}

void
coroapi::wait_for_tick_begin ()
{
  CALL_BEGIN (0);
  CALL_CALL ("cf::wait_for_tick_begin", G_DISCARD);
  CALL_END;
}

void
iow::poll (int events)
{
  if (events != this->events)
    {
      int active = ev_is_active ((ev_io *)this);
      if (active) stop ();
      ev_io_set ((ev_io *)this, fd, events);
      if (active) start ();
    }
}

static void
_connect_to_perl_1 ()
{
  stash_cf = gv_stashpv ("cf", 1);

  stash_cf_object_wrap        = gv_stashpv ("cf::object::wrap", 1);
  stash_cf_object_player_wrap = gv_stashpv ("cf::object::player::wrap", 1);
  stash_cf_player_wrap	      = gv_stashpv ("cf::player::wrap", 1);
  stash_cf_map_wrap           = gv_stashpv ("cf::map::wrap"   , 1);
  stash_cf_mapspace_wrap      = gv_stashpv ("cf::mapspace::wrap"   , 1);
  stash_cf_client_wrap        = gv_stashpv ("cf::client::wrap", 1);
  stash_cf_arch_wrap          = gv_stashpv ("cf::arch::wrap"  , 1);
  stash_cf_party_wrap         = gv_stashpv ("cf::party::wrap" , 1);
  stash_cf_region_wrap        = gv_stashpv ("cf::region::wrap", 1);
  stash_cf_living_wrap        = gv_stashpv ("cf::living::wrap", 1);
  stash_ext_map_world         = gv_stashpv ("ext::map_world"  , 1);

  sv_now        = get_sv ("cf::NOW"       , 1); SvUPGRADE (sv_now       , SVt_NV);
  sv_runtime    = get_sv ("cf::RUNTIME"   , 1); SvUPGRADE (sv_runtime   , SVt_NV);
  sv_tick_start = get_sv ("cf::TICK_START", 1); SvUPGRADE (sv_tick_start, SVt_NV);
  sv_next_tick  = get_sv ("cf::NEXT_TICK" , 1); SvUPGRADE (sv_next_tick , SVt_NV);

  cb_global      = get_av ("cf::CB_GLOBAL", 1);
  cb_attachable  = get_av ("cf::CB_ATTACHABLE", 1);
  cb_object      = get_av ("cf::CB_OBJECT", 1);
  cb_player      = get_av ("cf::CB_PLAYER", 1);
  cb_client      = get_av ("cf::CB_CLIENT", 1);
  cb_type        = get_av ("cf::CB_TYPE"  , 1);
  cb_map         = get_av ("cf::CB_MAP"   , 1);
}

static void
_connect_to_perl_2 ()
{
  cv_cf_do_invoke       = (SV *)get_cv ("cf::do_invoke"       , 0); assert (cv_cf_do_invoke);
  cv_cf__can_merge      = (SV *)get_cv ("cf::_can_merge"      , 0); assert (cv_cf__can_merge);
  cv_cf_client_send_msg = (SV *)get_cv ("cf::client::send_msg", 0); assert (cv_cf_client_send_msg);
  cv_cf_tick            = (SV *)get_cv ("cf::tick"            , 0); assert (cv_cf_tick);
  cv_cf_match_match     = (SV *)get_cv ("cf::match::match"    , 0); assert (cv_cf_match_match);
}

MODULE = cf             PACKAGE = cf         PREFIX = cf_

BOOT:
{
  I_EV_API   (PACKAGE); evapi::GEVAPI     = GEVAPI;
  I_CORO_API (PACKAGE); coroapi::GCoroAPI = GCoroAPI;

  _connect_to_perl_1 ();

  newCONSTSUB (stash_cf, "VERSION", newSVpv (VERSION, sizeof (VERSION) - 1));

  //{
  //  require_pv ("Time::HiRes");
  //
  //  SV **svp = hv_fetch (PL_modglobal, "Time::NVtime", 12, 0);
  //  if (!svp)         croak ("Time::HiRes is required");
  //  if (!SvIOK(*svp)) croak ("Time::NVtime isn’t a function pointer");
  //  coroapi::time = INT2PTR (double(*)(), SvIV(*svp));
  //}

  static const struct {
    const char *name;
    IV iv;
  } *civ, const_iv[] = {
#   define const_iv(name) { # name, (IV)name },
#   include "const_iv.h"
#   define def(uc, lc, name, plus, change) const_iv (AT_ ## uc) const_iv (ATNR_ ## uc)
#     include "attackinc.h"
#   undef def
#   define def(uc, flags) const_iv (SK_ ## uc)
#     include "skillinc.h"
#   undef def

    const_iv (Map0Cmd) const_iv (Map1Cmd) const_iv (Map1aCmd)

    const_iv (MAP_CLIENT_X) const_iv (MAP_CLIENT_Y)

    const_iv (MAX_TIME)
    const_iv (MAXSOCKBUF)

    const_iv (UPD_LOCATION)	const_iv (UPD_FLAGS)	const_iv (UPD_WEIGHT)		const_iv (UPD_FACE)
    const_iv (UPD_NAME)		const_iv (UPD_ANIM)	const_iv (UPD_ANIMSPEED)	const_iv (UPD_NROF)

    const_iv (UPD_SP_MANA)	const_iv (UPD_SP_GRACE)	const_iv (UPD_SP_LEVEL)

    const_iv (F_APPLIED)	const_iv (F_LOCATION)	const_iv (F_UNPAID)	const_iv (F_MAGIC)
    const_iv (F_CURSED)		const_iv (F_DAMNED)	const_iv (F_OPEN)	const_iv (F_NOPICK)
    const_iv (F_LOCKED)

    const_iv (P_BLOCKSVIEW)		const_iv (P_NO_MAGIC)		const_iv (P_IS_ALIVE)
    const_iv (P_NO_CLERIC)		const_iv (P_OUT_OF_MAP)		const_iv (P_NEW_MAP)		const_iv (P_UPTODATE)

    const_iv (SAVE_MODE)		const_iv (SAVE_DIR_MODE)

    const_iv (SK_EXP_ADD_SKILL)		const_iv (SK_EXP_TOTAL)		const_iv (SK_EXP_NONE)
    const_iv (SK_SUBTRACT_SKILL_EXP)	const_iv (SK_EXP_SKILL_ONLY)

    const_iv (MAP_ACTIVE)		const_iv (MAP_SWAPPED)		const_iv (MAP_LOADING)		const_iv (MAP_SAVING)
    const_iv (MAP_INACTIVE)

    const_iv (KLASS_ATTACHABLE)		const_iv (KLASS_GLOBAL)		const_iv (KLASS_OBJECT)
    const_iv (KLASS_CLIENT)		const_iv (KLASS_PLAYER)		const_iv (KLASS_MAP)

    const_iv (CS_QUERY_YESNO)		const_iv (CS_QUERY_SINGLECHAR)	const_iv (CS_QUERY_HIDEINPUT)

    const_iv (IO_HEADER)		const_iv (IO_OBJECTS)		const_iv (IO_UNIQUES)
  };

  for (civ = const_iv + array_length (const_iv); civ-- > const_iv; )
    newCONSTSUB (stash_cf, (char *)civ->name, newSViv (civ->iv));

  static const struct {
    const char *name;
    int skip;
    IV klass;
    IV iv;
  } *eiv, event_iv[] = {
#  define def(klass,name) { "EVENT_" # klass "_" # name, sizeof ("EVENT_" # klass), (IV)KLASS_ ## klass, (IV)EVENT_ ## klass ## _ ## name },
#   include "eventinc.h"
#  undef def
  };

  AV *av = get_av ("cf::EVENT", 1);

  for (eiv = event_iv + array_length (event_iv); eiv-- > event_iv; )
    {
      AV *event = newAV ();
      av_push (event, newSVpv ((char *)eiv->name + eiv->skip, 0));
      av_push (event, newSViv (eiv->klass));
      av_store (av, eiv->iv, newRV_noinc ((SV *)event));
      newCONSTSUB (stash_cf, (char *)eiv->name, newSViv (eiv->iv));
    }

  // used by autogenerated BOOT sections from genacc
  av_reflect = get_av ("cf::REFLECT", 1);
}

void _gv_clear (SV *gv)
	CODE:
        assert (SvTYPE (gv) == SVt_PVGV);
#	define f(sv) { SV *sv_ = (SV *)(sv); sv = 0; SvREFCNT_dec (sv_); }
        f (GvGP (gv)->gp_form);
        f (GvGP (gv)->gp_io);
        f (GvGP (gv)->gp_sv);
        f (GvGP (gv)->gp_av);
        f (GvGP (gv)->gp_hv);
        f (GvGP (gv)->gp_cv);
        GvCVGEN (gv) = 0;
        GvMULTI_off (gv);
#	undef f

void _connect_to_perl_1 ()

void _connect_to_perl_2 ()

void _recalc_want ()

# not used by default anymore
void _global_reattach ()
	CODE:
{
        // reattach to all attachable objects in the game.
        for_all_clients (ns)
          ns->reattach ();
        
        for_all_objects (op)
          op->reattach ();
}

# support function for map-world.ext
void _quantise (SV *data_sv, SV *plt_sv)
	CODE:
{
        if (!SvROK (plt_sv) || SvTYPE (SvRV (plt_sv)) != SVt_PVAV)
          croak ("_quantise called with invalid agruments");

        plt_sv = SvRV (plt_sv);
        SV **plt = AvARRAY (plt_sv);
        int plt_count = AvFILL (plt_sv) + 1;

	STRLEN len;
        char *data = SvPVbyte (data_sv, len);
        char *dst = data;

        while (len >= 3)
          {
            for (SV **val_sv = plt + plt_count; val_sv-- > plt; )
              {
                char *val = SvPVX (*val_sv);

                if (val [0] == data [0]
                    && val [1] == data [1]
                    && val [2] == data [2])
                  {
                    *dst++ = val [3];
                    goto next;
                  }
              }

            croak ("_quantise: color not found in palette: #%02x%02x%02x, at offset %d %d",
                   (uint8_t)data [0], (uint8_t)data [1], (uint8_t)data [2],
                   dst - SvPVX (data_sv), len);

            next:
            data += 3;
            len  -= 3;
          }

        SvCUR_set (data_sv, dst - SvPVX (data_sv));
}

void init_anim ()

void init_globals ()

void init_attackmess ()

void init_dynamic ()

void load_settings ()

void reload_exp_table ()

void reload_materials ()

void init_uuid ()
	CODE:
	UUID::init ();

void init_signals ()

void init_skills ()

void init_beforeplay ()

void evthread_start (int aiofd)

void cede_to_tick ()
	CODE:
        coroapi::cede_to_tick ();

NV till_tick ()
	CODE:
        RETVAL = SvNVX (sv_next_tick) - now ();
	OUTPUT:
        RETVAL

int tick_inhibit ()
	CODE:
        RETVAL = tick_inhibit;
	OUTPUT:
        RETVAL

void tick_inhibit_inc ()
	CODE:
        ++tick_inhibit;

void tick_inhibit_dec ()
	CODE:
        if (!--tick_inhibit)
          if (tick_pending)
            {
              ev_async_send (EV_DEFAULT, &tick_watcher);
              coroapi::cede ();
            }

void server_tick ()
	CODE:
{
        ev_now_update (EV_DEFAULT);
        NOW = ev_now (EV_DEFAULT);
        SvNV_set (sv_now, NOW); SvNOK_only (sv_now);
        SvNV_set (sv_tick_start, NOW); SvNOK_only (sv_tick_start);
        runtime = SvNVX (sv_runtime);

        server_tick ();

        ev_now_update (EV_DEFAULT);
        NOW = ev_now (EV_DEFAULT);
        SvNV_set (sv_now, NOW); SvNOK_only (sv_now);
        runtime += TICK;
        SvNV_set (sv_runtime, runtime); SvNOK_only (sv_runtime);
}

NV floor (NV x)

NV ceil (NV x)

NV rndm (...)
	ALIAS:
        rmg_rndm = 1
	CODE:
{
  	rand_gen &gen = ix ? rmg_rndm : rndm;
        switch (items)
          {
            case 0: RETVAL = gen (); break;
            case 1: RETVAL = gen (SvUV (ST (0))); break;
            case 2: RETVAL = gen (SvIV (ST (0)), SvIV (ST (1))); break;
            default: croak ("cf::rndm requires zero, one or two parameters."); break;
          }
}
        OUTPUT:
        RETVAL

NV clamp (NV value, NV min_value, NV max_value)
	CODE:
        RETVAL = clamp (value, min_value, max_value);
        OUTPUT:
        RETVAL

NV lerp (NV value, NV min_in, NV max_in, NV min_out, NV max_out)
	CODE:
        RETVAL = lerp (value, min_in, max_in, min_out, max_out);
        OUTPUT:
        RETVAL

const char *ordinal (int i)

void weaken (...)
        PROTOTYPE: @
        CODE:
        while (items > 0)
          sv_rvweaken (ST (--items));

void log_suspend ()

void log_resume ()

void log_backtrace (utf8_string msg)

void LOG (int flags, utf8_string msg)
	PROTOTYPE: $$
	C_ARGS: flags, "%s", msg

octet_string path_combine (octet_string base, octet_string path)
	PROTOTYPE: $$

octet_string path_combine_and_normalize (octet_string base, octet_string path)
	PROTOTYPE: $$

void
sub_generation_inc ()
	CODE:
        PL_sub_generation++;

const_octet_string
mapdir ()
	PROTOTYPE:
	ALIAS:
        mapdir    = 0
        uniquedir = 1
        tmpdir    = 2
        confdir   = 3
        localdir  = 4
        playerdir = 5
        datadir   = 6
        CODE:
        switch (ix)
          {
            case 0: RETVAL = settings.mapdir   ; break; 
            case 1: RETVAL = settings.uniquedir; break; 
            case 2: RETVAL = settings.tmpdir   ; break; 
            case 3: RETVAL = settings.confdir  ; break; 
            case 4: RETVAL = settings.localdir ; break; 
            case 5: RETVAL = settings.playerdir; break; 
            case 6: RETVAL = settings.datadir  ; break;
          }
	OUTPUT: RETVAL

void abort ()

void reset_signals ()

void fork_abort (const_octet_string cause = "cf::fork_abort")

void cleanup (const_octet_string cause, bool make_core = false)

void emergency_save ()

void _exit (int status = EXIT_SUCCESS)

#if _POSIX_MEMLOCK

int mlockall (int flags = MCL_CURRENT | MCL_FUTURE)
	INIT:
#if __GLIBC__
        mallopt (M_TOP_PAD, 1024 * 1024);
        mallopt (M_MMAP_THRESHOLD, 1024 * 1024 * 128);
        mallopt (M_MMAP_MAX, 0); // likely bug-workaround, also frees memory
        mallopt (M_PERTURB, 0xee); // bug-workaround for linux glibc+mlockall+calloc
#endif

int munlockall ()

#endif

int
malloc_trim (IV pad = 0)

void
mallinfo ()
	PPCODE:
{
#if __GLIBC__
	struct mallinfo mai = mallinfo ();
        EXTEND (SP, 10*2);
        PUSHs (sv_2mortal (newSVpv ("arena"   , 0))); PUSHs (sv_2mortal (newSViv (mai.arena)));
        PUSHs (sv_2mortal (newSVpv ("ordblks" , 0))); PUSHs (sv_2mortal (newSViv (mai.ordblks)));
        PUSHs (sv_2mortal (newSVpv ("smblks"  , 0))); PUSHs (sv_2mortal (newSViv (mai.smblks)));
        PUSHs (sv_2mortal (newSVpv ("hblks"   , 0))); PUSHs (sv_2mortal (newSViv (mai.hblks)));
        PUSHs (sv_2mortal (newSVpv ("hblkhd"  , 0))); PUSHs (sv_2mortal (newSViv (mai.hblkhd)));
        PUSHs (sv_2mortal (newSVpv ("usmblks" , 0))); PUSHs (sv_2mortal (newSViv (mai.usmblks)));
        PUSHs (sv_2mortal (newSVpv ("fsmblks" , 0))); PUSHs (sv_2mortal (newSViv (mai.fsmblks)));
        PUSHs (sv_2mortal (newSVpv ("uordblks", 0))); PUSHs (sv_2mortal (newSViv (mai.uordblks)));
        PUSHs (sv_2mortal (newSVpv ("fordblks", 0))); PUSHs (sv_2mortal (newSViv (mai.fordblks)));
        PUSHs (sv_2mortal (newSVpv ("keepcost", 0))); PUSHs (sv_2mortal (newSViv (mai.keepcost)));
#endif
        EXTEND (SP, 5*2);
        PUSHs (sv_2mortal (newSVpv ("slice_alloc", 0))); PUSHs (sv_2mortal (newSVuv (slice_alloc)));
        PUSHs (sv_2mortal (newSVpv ("shstr_alloc", 0))); PUSHs (sv_2mortal (newSVuv (shstr_alloc)));
        PUSHs (sv_2mortal (newSVpv ("objects"    , 0))); PUSHs (sv_2mortal (newSVuv (objects.size () * sizeof (object))));
        PUSHs (sv_2mortal (newSVpv ("sv_count"   , 0))); PUSHs (sv_2mortal (newSVuv (PL_sv_count)));
        PUSHs (sv_2mortal (newSVpv ("sv_objcount", 0))); PUSHs (sv_2mortal (newSVuv (PL_sv_objcount)));
}

int find_animation (utf8_string text)
	PROTOTYPE: $

int random_roll (int min, int max, object *op, int goodbad);

const_utf8_string cost_string_from_value(uint64 cost, int approx = 0)

int exp_to_level (val64 exp)

val64 level_to_min_exp (int level)

SV *
resistance_to_string (int atnr)
        CODE:
        if (atnr >= 0 && atnr < NROFATTACKS)
          RETVAL = newSVpv (resist_plus[atnr], 0);
        else
          XSRETURN_UNDEF;
        OUTPUT: RETVAL

UUID
uuid_cur ()
	CODE:
        RETVAL = UUID::cur;
	OUTPUT:
        RETVAL

UUID
uuid_gen ()
	CODE:
        RETVAL = UUID::gen ();
	OUTPUT:
        RETVAL

val64
uuid_seq (UUID uuid)
	CODE:
        RETVAL = uuid.seq;
	OUTPUT:
        RETVAL

UUID
uuid_str (val64 seq)
	CODE:
        RETVAL.seq = seq;
	OUTPUT:
        RETVAL

void
coin_names ()
	PPCODE:
        EXTEND (SP, NUM_COINS);
        for (int i = 0; i < NUM_COINS; ++i)
           PUSHs (sv_2mortal (newSVpv (coins [i], 0)));

void
coin_archetypes ()
	PPCODE:
        EXTEND (SP, NUM_COINS);
        for (int i = 0; i < NUM_COINS; ++i)
           PUSHs (sv_2mortal (to_sv (archetype::find (coins [i]))));

bool
load_resource_file_ (octet_string filename)

void
fix_weight ()

MODULE = cf        PACKAGE = cf::attachable

int
valid (SV *obj)
	CODE:
        RETVAL = SvROK (obj) && mg_find (SvRV (obj), PERL_MAGIC_ext);
	OUTPUT:
	RETVAL

bool should_invoke (attachable *obj, int event)
	CODE:
        RETVAL = obj->should_invoke ((event_type)event);
	OUTPUT: RETVAL

void
debug_trace (attachable *obj, bool on = true)
	CODE:
        obj->attachable_flags &= ~attachable::F_DEBUG_TRACE;
        if (on)
          obj->attachable_flags |= attachable::F_DEBUG_TRACE;

int mortals_size ()
	CODE:
        RETVAL = attachable::mortals.size ();
	OUTPUT: RETVAL

#object *mortals (U32 index)
#	CODE:
#        RETVAL = index < attachable::mortals.size () ? attachable::mortals [index] : 0;
#	OUTPUT: RETVAL

INCLUDE: $PERL $srcdir/genacc attachable $srcdir/../include/util.h $srcdir/../include/cfperl.h |

MODULE = cf        PACKAGE = cf::global

int invoke (SV *klass, int event, ...)
	CODE:
        if (KLASS_OF (event) != KLASS_GLOBAL) croak ("event class must be GLOBAL");
	AV *av = (AV *)sv_2mortal ((SV *)newAV ());
        for (int i = 1; i < items; i++) av_push (av, SvREFCNT_inc (ST (i)));
        RETVAL = gbl_ev.invoke ((event_type)event, ARG_AV (av), DT_END);
	OUTPUT: RETVAL

MODULE = cf        PACKAGE = cf::object         PREFIX = cf_object_

INCLUDE: $PERL $srcdir/genacc object $srcdir/../include/object.h |

int invoke (object *op, int event, ...)
	CODE:
        if (KLASS_OF (event) != KLASS_OBJECT) croak ("event class must be OBJECT");
	AV *av = (AV *)sv_2mortal ((SV *)newAV ());
        for (int i = 2; i < items; i++) av_push (av, SvREFCNT_inc (ST (i)));
        RETVAL = op->invoke ((event_type)event, ARG_AV (av), DT_END);
	OUTPUT: RETVAL

SV *registry (object *op)

int objects_size ()
	CODE:
        RETVAL = objects.size ();
	OUTPUT: RETVAL

object *objects (U32 index)
	CODE:
        RETVAL = index < objects.size () ? objects [index] : 0;
	OUTPUT: RETVAL

int actives_size ()
	CODE:
        RETVAL = actives.size ();
	OUTPUT: RETVAL

object *actives (U32 index)
	CODE:
        RETVAL = index < actives.size () ? actives [index] : 0;
	OUTPUT: RETVAL

int mortals_size ()
	CODE:
        RETVAL = attachable::mortals.size ();
	OUTPUT: RETVAL

const_utf8_string slot_use_name (U32 slot)
	ALIAS:
        slot_nonuse_name = 1
        CODE:
{
        if (slot >= NUM_BODY_LOCATIONS)
          croak ("body slot index out of range");

        switch (ix)
          {
            case 0: RETVAL = body_locations[slot].use_name;    break;
            case 1: RETVAL = body_locations[slot].nonuse_name; break;
          }
}
	OUTPUT:
        RETVAL

# missing properties

object *head (object *op)
	PROTOTYPE: $
	CODE:
        RETVAL = op->head_ ();
        OUTPUT: RETVAL

void
inv (object *obj)
	PROTOTYPE: $
        PPCODE:
{
	for (object *o = obj->inv; o; o = o->below)
          XPUSHs (sv_2mortal (to_sv (o)));
}

void
set_animation (object *op, int idx)
	CODE:
        SET_ANIMATION (op, idx);

int
num_animations (object *op)
	CODE:
        RETVAL = NUM_ANIMATIONS (op);
        OUTPUT: RETVAL

int slot_info (object *op, UV slot, int value = 0)
	ALIAS:
        slot_used = 1
	CODE:
{
        if (slot >= NUM_BODY_LOCATIONS)
          croak ("body slot index out of range");

        RETVAL = ix ? op->slot[slot].used : op->slot[slot].info;

        if (items > 2)
          if (ix)
            op->slot[slot].used = value;
          else
            op->slot[slot].info = value;
}
        OUTPUT:
        RETVAL

object *find_best_object_match (object *op, utf8_string match)

int apply_shop_mat (object *shop_mat, object *op);

int move (object *op, int dir, object *originator = op)
	CODE:
        RETVAL = op->move (dir, originator);
	OUTPUT:
        RETVAL

void apply_below (object *op)
	CODE:
        player_apply_below (op);

int cast_heal (object *op, object *caster, object *spell, int dir = 0)

int casting_level (object *caster, object *spell)

int pay_item (object *op, object *buyer)
	CODE:
        RETVAL = pay_for_item (op, buyer);
	OUTPUT: RETVAL

int pay_amount (object *op, uint64 amount)
	CODE:
        RETVAL = pay_for_amount (amount, op);
	OUTPUT: RETVAL

void pay_player (object *op, uint64 amount)

val64 pay_player_arch (object *op, utf8_string arch, uint64 amount)

int cast_spell (object *op, object *caster, int dir, object *spell_ob, utf8_string stringarg = 0)

void learn_spell (object *op, object *sp, int special_prayer = 0)
	CODE:
        do_learn_spell (op, sp, special_prayer);

void forget_spell (object *op, object *sp)
	CODE:
        do_forget_spell (op, query_name (sp));

object *check_for_spell (object *op, utf8_string spellname)
	CODE:
        RETVAL = check_spell_known (op, spellname);
	OUTPUT: RETVAL

int query_money (object *op)
	ALIAS: money = 0

val64 query_cost (object *op, object *who, int flags)
	ALIAS: cost = 0

void spring_trap (object *op, object *victim)

int check_trigger (object *op, object *cause)

void drop (object *who, object *op)

void pick_up (object *who, object *op)

void update_object (object *op, int action)

void change_exp (object *op, uint64 exp, shstr_tmp skill_name = shstr_tmp (), int flag = 0)

void player_lvl_adj (object *who, object *skill = 0)

int kill_object (object *op, int dam = 0, object *hitter = 0, int type = AT_PHYSICAL)

int calc_skill_exp (object *who, object *op, object *skill)

void push_button (object *op, object *originator)

void use_trigger (object *op, object *originator)

void handle_apply_yield (object *op)

int convert_item (object *item, object *converter)

void fix_generated_item (object *op, object *creator, int difficulty, int max_magic, int flags);

MODULE = cf        PACKAGE = cf::object         PREFIX = cf_

# no clean way to get an object from an archetype - stupid idiotic
# dumb kludgy misdesigned plug-in api slowly gets on my nerves.

object *new (utf8_string archetype = 0)
	PROTOTYPE: ;$
        CODE:
        RETVAL = archetype ? get_archetype (archetype) : object::create ();
        OUTPUT:
	RETVAL

object *generate (utf8_string arch, object *creator)
	CODE:
        object *obj  = get_archetype (arch);
        fix_generated_item (obj, creator, 0, 0, GT_MINIMAL);
        RETVAL = obj;
        OUTPUT:
	RETVAL

object *find_object (U32 tag)

object *find_object_uuid (UUID i)

# TODO: nuke
object *insert_ob_in_map_at (object *ob, maptile *where, object_ornull *orig, int flag, int x, int y)
	PROTOTYPE: $$$$$$
        CODE:
{
        RETVAL = insert_ob_in_map_at (ob, where, orig, flag, x, y);

        if (RETVAL->destroyed ())
          RETVAL = 0;
}

shstr
object::kv_get (shstr key)

void
object::kv_del (shstr key)

void
object::kv_set (shstr key, shstr value)

object *get_nearest_player (object *ob)
	ALIAS: nearest_player = 0
        PREINIT:
        extern object *get_nearest_player (object *);

void rangevector (object *ob, object *other, int flags = 0)
	PROTOTYPE: $$;$
        PPCODE:
{
        rv_vector rv;

        get_rangevector (ob, other, &rv, flags);

        EXTEND (SP, 5);
        PUSHs (sv_2mortal (newSVuv (rv.distance)));
        PUSHs (sv_2mortal (newSViv (rv.distance_x)));
        PUSHs (sv_2mortal (newSViv (rv.distance_y)));
        PUSHs (sv_2mortal (newSViv (rv.direction)));
        PUSHs (sv_2mortal (to_sv   (rv.part)));
}

bool on_same_map_as (object *ob, object *other)
	CODE:
        RETVAL = on_same_map (ob, other);
	OUTPUT: RETVAL

const_utf8_string
base_name (object *op, int plural = op->nrof > 1)
        CODE:
        RETVAL = query_base_name (op, plural);
        OUTPUT: RETVAL

# return the tail of an object, excluding itself
void
tail (object *op)
	PPCODE:
        while ((op = op->more))
          XPUSHs (sv_2mortal (to_sv (op)));

MODULE = cf        PACKAGE = cf::object::player PREFIX = cf_player_

player *player (object *op)
	CODE:
        RETVAL = op->contr;
	OUTPUT: RETVAL

bool move_player (object *op, int dir)

void message (object *op, utf8_string txt, int flags = NDI_ORANGE | NDI_UNIQUE)
	CODE:
        new_draw_info (flags, 0, op, txt);

void kill_player (object *op)

void esrv_send_item (object *pl, object *item)

void esrv_update_item (object *pl, int what, object *item)
        C_ARGS: what, pl, item

void esrv_del_item (object *pl, int tag)
        C_ARGS: pl->contr, tag

int command_summon (object *op, utf8_string params)

int command_arrest (object *op, utf8_string params)


MODULE = cf        PACKAGE = cf::player         PREFIX = cf_player_

INCLUDE: $PERL $srcdir/genacc player $srcdir/../include/player.h |

int invoke (player *pl, int event, ...)
	CODE:
        if (KLASS_OF (event) != KLASS_PLAYER) croak ("event class must be PLAYER");
	AV *av = (AV *)sv_2mortal ((SV *)newAV ());
        for (int i = 2; i < items; i++) av_push (av, SvREFCNT_inc (ST (i)));
        RETVAL = pl->invoke ((event_type)event, ARG_AV (av), DT_END);
	OUTPUT: RETVAL

SV *registry (player *pl)

void
save_stats (player *pl)
	CODE:
        pl->ob->stats.hp    = pl->ob->stats.maxhp;
        pl->ob->stats.sp    = pl->ob->stats.maxsp;
        pl->ob->stats.grace = pl->ob->stats.maxgrace;
        pl->orig_stats      = pl->ob->stats;

# should only be temporary
void esrv_new_player (player *pl)

#d# TODO: replace by blocked_los accessor, fix code using this
bool
cell_visible (player *pl, int dx, int dy)
	CODE:
        RETVAL = pl->blocked_los (dx, dy) != LOS_BLOCKED;
        OUTPUT:
        RETVAL

void
send (player *pl, SV *packet)
	CODE:
{
	STRLEN len;
	char *buf = SvPVbyte (packet, len);

        if (len > MAXSOCKBUF)
          pl->failmsg ("[packet too long for client]");
        else if (pl->ns)
          pl->ns->send_packet (buf, len);
}

void savebed (player *pl, SV *map_path = 0, SV *x = 0, SV *y = 0)
	PROTOTYPE: $;$$$
	PPCODE:
        if (GIMME_V != G_VOID)
          {
            EXTEND (SP, 3);
            PUSHs (sv_2mortal (newSVpv (pl->savebed_map, 0)));
            PUSHs (sv_2mortal (newSViv (pl->bed_x)));
            PUSHs (sv_2mortal (newSViv (pl->bed_y)));
          }
        if (map_path) sv_to (map_path, pl->savebed_map);
        if (x)        sv_to (x,        pl->bed_x);
        if (y)        sv_to (y,        pl->bed_y);

void
list ()
	PPCODE:
        for_all_players (pl)
          XPUSHs (sv_2mortal (to_sv (pl)));


MODULE = cf        PACKAGE = cf::map            PREFIX = cf_map_

int invoke (maptile *map, int event, ...)
	CODE:
        if (KLASS_OF (event) != KLASS_MAP) croak ("event class must be MAP");
	AV *av = (AV *)sv_2mortal ((SV *)newAV ());
        for (int i = 2; i < items; i++) av_push (av, SvREFCNT_inc (ST (i)));
        RETVAL = map->invoke ((event_type)event, ARG_AV (av), DT_END);
	OUTPUT: RETVAL

SV *registry (maptile *map)

void
find_tagged_objects (maptile *map, utf8_string tag = 0)
	PPCODE:
{
        if (!map->spaces)
          XSRETURN_EMPTY;

	if (tag)
          {
            shstr_cmp tag_ (tag);

            for (mapspace *ms = map->spaces + map->size (); ms-- > map->spaces; )
              for (object *op = ms->bot; op; op = op->above)
                if (op->tag == tag_)
                  XPUSHs (sv_2mortal (to_sv (op)));
          }
        else
          {
            for (mapspace *ms = map->spaces + map->size (); ms-- > map->spaces; )
              for (object *op = ms->bot; op; op = op->above)
                if (op->tag)
                  XPUSHs (sv_2mortal (to_sv (op)));
          }
}

INCLUDE: $PERL $srcdir/genacc maptile $srcdir/../include/map.h |

void
adjust_daylight ()
	CODE:
        maptile::adjust_daylight ();

int
outdoor_darkness (int darkness = 0)
	CODE:
        RETVAL = maptile::outdoor_darkness;
        if (items)
          maptile::outdoor_darkness = darkness;
	OUTPUT:
        RETVAL

void
maptile::instantiate ()

maptile *new ()
	PROTOTYPE:
	CODE:
        RETVAL = new maptile;
	OUTPUT:
        RETVAL

void
maptile::players ()
	PPCODE:
        if (GIMME_V == G_SCALAR)
          XPUSHs (sv_2mortal (to_sv (THIS->players)));
        else if (GIMME_V == G_ARRAY)
          {
            EXTEND (SP, THIS->players);
            for_all_players (pl)
              if (pl->ob && pl->ob->map == THIS)
                PUSHs (sv_2mortal (to_sv (pl->ob)));
          }

void
maptile::add_underlay (SV *data, int offset, int stride, SV *palette)
	CODE:
{
        if (!SvROK (palette) || SvTYPE (SvRV (palette)) != SVt_PVAV)
          croak ("maptile::add_underlay: palette must be arrayref");

        palette = SvRV (palette);

        STRLEN idxlen;
        const uint8_t *idx = (const uint8_t *)SvPVbyte (data, idxlen);

        for (int x = 0; x < THIS->width; ++x)
          for (int y = 0; y < THIS->height; ++y)
            {
              for (object *op = THIS->at (x, y).bot; op; op = op->above)
                if (op->flag [FLAG_IS_FLOOR])
                  goto skip;

              {
                int offs = offset + y * stride + x;

                if (IN_RANGE_EXC (offs, 0, idxlen))
                  {
                    if (SV **elem = av_fetch ((AV *)palette, idx [offs], 0))
                      {
                        object *ob = get_archetype (cfSvPVutf8_nolen (*elem));
                        ob->flag [FLAG_NO_MAP_SAVE] = true;
                        THIS->insert (ob, x, y, 0, INS_ABOVE_FLOOR_ONLY);

                        if (ob->randomitems && !ob->above)
                          {
                            ob->create_treasure (ob->randomitems);

                            for (object *op = ob->above; op; op = op->above)
                              op->flag [FLAG_NO_MAP_SAVE] = true;
                              // TODO: if this is a pickable object, then the item
                              // will at a bit weird - saving inside the player
                              // will clear the flag, but when the player drops
                              // it without logging out, it keeps the flag.
                              // nobody ahs reported this, but this can be rather
                              // annoying on persistent maps.
                          }
                      }
                  }
              }

              skip: ;
            }
}

void
maptile::set_regiondata (SV *data, int offset, int stride, SV *palette)
	CODE:
{
        if (!SvROK (palette) || SvTYPE (SvRV (palette)) != SVt_PVAV)
          croak ("maptile::set_regiondata: palette must be arrayref");

        palette = SvRV (palette);

        STRLEN idxlen;
        const uint8_t *idx = (const uint8_t *)SvPVbyte (data, idxlen);

        region_ptr *regionmap = new region_ptr [av_len ((AV *)palette) + 1];
        uint8_t *regions = salloc<uint8_t> (THIS->size ());

        for (int i = av_len ((AV *)palette) + 1; i--; )
          regionmap [i] = region::find (cfSvPVutf8_nolen (*av_fetch ((AV *)palette, i, 1)));

        for (int y = 0; y < THIS->height; ++y)
          memcpy (regions + y * THIS->width, idx + offset + y * stride, THIS->width);

        sfree (THIS->regions, THIS->size ());
        delete [] THIS->regionmap;

        THIS->regions   = regions;
        THIS->regionmap = regionmap;
}

void
maptile::create_region_treasure ()
	CODE:
        for (int x = 0; x < THIS->width; ++x)
          for (int y = 0; y < THIS->height; ++y)
            {
              region *rgn = THIS->region (x, y);

              //fprintf (stderr, "%d,%d %f %p\n", x, y, rgn->treasure_density,rgn->treasure);//D
              if (object *op = THIS->at (x, y).top)
                if (rgn->treasure && rndm () < rgn->treasure_density)
                  create_treasure (rgn->treasure, op, GT_ENVIRONMENT, THIS->difficulty);
            }

int out_of_map (maptile *map, int x, int y)

void
find_link (maptile *map, shstr_tmp connection)
       PPCODE:
       if (oblinkpt *obp = map->find_link (connection))
         for (objectlink *ol = obp->link; ol; ol = ol->next)
           XPUSHs (sv_2mortal (to_sv ((object *)ol->ob)));

void
xy_normalise (maptile *map, int x, int y, int dir = 0)
	PPCODE:
{
	mapxy pos (map, x, y);
        if (!pos.move (dir).normalise ())
          XSRETURN_EMPTY;

        EXTEND (SP, 3);
        PUSHs (sv_2mortal (to_sv (pos.m)));
        PUSHs (sv_2mortal (to_sv (pos.x)));
        PUSHs (sv_2mortal (to_sv (pos.y)));
}

mapspace *
ms (maptile *map, unsigned int x, unsigned int y, int dir = 0)
	PROTOTYPE: $$$;$
        CODE:
{
	mapxy pos (map, x, y);
        if (!pos.move (dir).normalise ())
          XSRETURN_UNDEF;

        RETVAL = &*pos;
}
	OUTPUT:
        RETVAL

void
at (maptile *map, unsigned int x, unsigned int y, int dir = 0)
	PROTOTYPE: $$$;$
        PPCODE:
	mapxy pos (map, x, y);
        if (pos.move (dir).normalise ())
          for (object *o = pos->bot; o; o = o->above)
            XPUSHs (sv_2mortal (to_sv (o)));

SV *
bot_at (maptile *map, unsigned int x, unsigned int y, int dir = 0)
	PROTOTYPE: $$$;$
        ALIAS:
          top_at        = 1
          flags_at      = 2
          light_at      = 3
          move_block_at = 4
          move_slow_at  = 5
          move_on_at    = 6
          move_off_at   = 7
        CODE:
{
	mapxy pos (map, x, y);
        if (!pos.move (dir).normalise ())
	  XSRETURN_UNDEF;

        mapspace &ms = *pos;

        ms.update ();

        switch (ix)
          {
            case 0: RETVAL = to_sv   (ms.bot       ); break;
            case 1: RETVAL = to_sv   (ms.top       ); break;
            case 2: RETVAL = newSVuv (ms.flags_    ); break;
            case 3: RETVAL = newSViv (ms.light     ); break;
            case 4: RETVAL = newSVuv (ms.move_block); break;
            case 5: RETVAL = newSVuv (ms.move_slow ); break;
            case 6: RETVAL = newSVuv (ms.move_on   ); break;
            case 7: RETVAL = newSVuv (ms.move_off  ); break;
          }
}
        OUTPUT: RETVAL

# worst xs function of my life
bool
_create_random_map (\
        maptile *self,\
	utf8_string wallstyle,\
	utf8_string wall_name,\
	utf8_string floorstyle,\
	utf8_string monsterstyle,\
	utf8_string treasurestyle,\
	utf8_string layoutstyle,\
	utf8_string doorstyle,\
	utf8_string decorstyle,\
	utf8_string miningstyle,\
	utf8_string origin_map,\
	utf8_string final_map,\
	utf8_string exitstyle,\
	utf8_string this_map,\
	utf8_string exit_on_final_map,\
	int xsize,\
	int ysize,\
	int expand2x,\
	int layoutoptions1,\
	int layoutoptions2,\
	int layoutoptions3,\
	int symmetry,\
	int difficulty,\
	int difficulty_given,\
	float difficulty_increase,\
	int dungeon_level,\
	int dungeon_depth,\
	int decoroptions,\
	int orientation,\
	int origin_y,\
	int origin_x,\
	U32 random_seed,\
	val64 total_map_hp,\
	int map_layout_style,\
	int treasureoptions,\
	int symmetry_used,\
	region *region,\
        utf8_string custom\
)
	CODE:
{
  	random_map_params rmp;

        assign (rmp.wallstyle        , wallstyle);
        assign (rmp.wall_name        , wall_name);
        assign (rmp.floorstyle       , floorstyle);
        assign (rmp.monsterstyle     , monsterstyle);
        assign (rmp.treasurestyle    , treasurestyle);
        assign (rmp.layoutstyle      , layoutstyle);
        assign (rmp.doorstyle        , doorstyle);
        assign (rmp.decorstyle       , decorstyle);
        assign (rmp.miningstyle      , miningstyle);
        assign (rmp.exitstyle        , exitstyle);
        assign (rmp.exit_on_final_map, exit_on_final_map);

        rmp.origin_map          = origin_map;
        rmp.final_map           = final_map;
        rmp.this_map            = this_map;
        rmp.xsize               = xsize;
        rmp.ysize               = ysize;
        rmp.expand2x            = expand2x;
        rmp.layoutoptions1      = layoutoptions1;
        rmp.layoutoptions2      = layoutoptions2;
        rmp.layoutoptions3      = layoutoptions3;
        rmp.symmetry            = symmetry;
        rmp.difficulty          = difficulty;
        rmp.difficulty_given    = difficulty_given;
        rmp.difficulty_increase = difficulty_increase;
        rmp.dungeon_level       = dungeon_level;
        rmp.dungeon_depth       = dungeon_depth;
        rmp.decoroptions        = decoroptions;
        rmp.orientation         = orientation;
        rmp.origin_y            = origin_y;
        rmp.origin_x            = origin_x;
        rmp.random_seed         = random_seed;
        rmp.total_map_hp        = (uint64_t) total_map_hp;
        rmp.map_layout_style    = map_layout_style;
        rmp.treasureoptions     = treasureoptions;
        rmp.symmetry_used       = symmetry_used;
        rmp.region              = region;
        rmp.custom		= custom;

        RETVAL = self->generate_random_map (&rmp);
}
	OUTPUT:
        RETVAL

MODULE = cf        PACKAGE = cf::mapspace

INCLUDE: $PERL $srcdir/genacc mapspace $srcdir/../include/map.h |

MODULE = cf        PACKAGE = cf::arch

int archetypes_size ()
	CODE:
        RETVAL = archetypes.size ();
	OUTPUT: RETVAL

archetype *archetypes (U32 index)
	CODE:
        RETVAL = index < archetypes.size () ? archetypes [index] : 0;
	OUTPUT: RETVAL

INCLUDE: $PERL $srcdir/genacc archetype $srcdir/../include/object.h |

MODULE = cf        PACKAGE = cf::party

partylist *first ()
	PROTOTYPE:
        CODE:
        RETVAL = get_firstparty ();
        OUTPUT: RETVAL

INCLUDE: $PERL $srcdir/genacc partylist $srcdir/../include/player.h |

MODULE = cf        PACKAGE = cf::region

void
list ()
	PPCODE:
        for_all_regions (rgn)
          XPUSHs (sv_2mortal (to_sv (rgn)));

int specificity (region *rgn)
	CODE:
        RETVAL = 0;
        while (rgn = rgn->parent)
          RETVAL++;
        OUTPUT: RETVAL

INCLUDE: $PERL $srcdir/genacc region $srcdir/../include/region.h |

MODULE = cf        PACKAGE = cf::living

INCLUDE: $PERL $srcdir/genacc living $srcdir/../include/living.h |

MODULE = cf        PACKAGE = cf::settings

INCLUDE: $PERL $srcdir/genacc Settings $srcdir/../include/global.h |

MODULE = cf        PACKAGE = cf::client

INCLUDE: $PERL $srcdir/genacc client $srcdir/../include/client.h |

int invoke (client *ns, int event, ...)
	CODE:
        if (KLASS_OF (event) != KLASS_CLIENT) croak ("event class must be CLIENT");
	AV *av = (AV *)sv_2mortal ((SV *)newAV ());
        for (int i = 2; i < items; i++) av_push (av, SvREFCNT_inc (ST (i)));
        RETVAL = ns->invoke ((event_type)event, ARG_AV (av), DT_END);
	OUTPUT: RETVAL

SV *registry (client *ns)

void
list ()
	PPCODE:
        EXTEND (SP, clients.size ());
        for (sockvec::iterator i = clients.begin (); i != clients.end (); ++i)
          PUSHs (sv_2mortal (to_sv (*i)));

void
client::send_packet (SV *packet)
	CODE:
{
	STRLEN len;
	char *buf = SvPVbyte (packet, len);

        if (len > MAXSOCKBUF)
          {
            // ugly
            if (THIS->pl)
              THIS->pl->failmsg ("[packet too long for client]");
          }
        else
          THIS->send_packet (buf, len);
}

faceidx
client::need_face (utf8_string name, int pri = 0)
	CODE:
        RETVAL = face_find (name, 0);
        if (RETVAL)
          {
            THIS->send_face (RETVAL, pri);
            THIS->flush_fx ();
          }
	OUTPUT:
        RETVAL

int
client::fx_want (int idx, int value = -1)
	CODE:
        if (0 < idx && idx < FT_NUM)
          {
            RETVAL = THIS->fx_want [idx];
            if (items > 2)
              THIS->fx_want [idx] = value;
          }
        else
          RETVAL = 0;
	OUTPUT:
        RETVAL

MODULE = cf        PACKAGE = cf::sound	PREFIX = sound_

faceidx sound_find (utf8_string name)

void sound_set (utf8_string str, faceidx face)

# dire hack
void old_sound_index (int idx, faceidx face)
	CODE:
        extern faceidx old_sound_index [SOUND_CAST_SPELL_0];
        old_sound_index [idx] = face;

MODULE = cf        PACKAGE = cf::face	PREFIX = face_

#INCLUDE: $PERL $srcdir/genacc faceset $srcdir/../include/face.h |

faceidx face_find (utf8_string name, faceidx defidx = 0)

faceidx alloc (utf8_string name)
	CODE:
{
        do
          {
            RETVAL = faces.size ();
            faces.resize (RETVAL + 1);
          }
        while (!RETVAL); // crude way to leave index 0

        faces [RETVAL].name = name;
        facehash.insert (std::make_pair (faces [RETVAL].name, RETVAL));

        if (!strcmp (name, BLANK_FACE_NAME     )) blank_face      = RETVAL;
        if (!strcmp (name, EMPTY_FACE_NAME     )) empty_face      = RETVAL;
        if (!strcmp (name, MAGICMOUTH_FACE_NAME)) magicmouth_face = RETVAL;
}
	OUTPUT: RETVAL

void set_type (faceidx idx, int value)
	ALIAS:
        set_type        = 0
        set_visibility  = 1
        set_magicmap    = 2
        set_smooth      = 3
        set_smoothlevel = 4
	CODE:
        faceinfo *f = face_info (idx); assert (f);
        switch (ix)
          {
            case 0: f->type        = value; break;
            case 1: f->visibility  = value; break;
            case 2: f->magicmap    = value; break;
            case 3: f->smooth      = value; break;
            case 4: f->smoothlevel = value; break;
          }

void set_data (faceidx idx, int faceset, SV *data, SV *chksum)
	CODE:
{
        faceinfo *f = face_info (idx); assert (f);
        facedata *d = &(faceset ? f->data64 : f->data32);
        sv_to (data, d->data);
        STRLEN clen;
        char *cdata = SvPVbyte (chksum, clen);
        clen = min (CHKSUM_MAXLEN, clen);

        assert (("cf::face::set_data must be called with a non-empty checksum", clen));

        if (clen != d->chksum_len || memcmp (d->chksum, cdata, clen))
          {
            d->chksum_len = clen;
            memcpy (d->chksum, cdata, clen);

            // invalidate existing client face info
            for_all_clients (ns)
              if (ns->faceset == faceset)
                {
                  ns->faces_sent [idx] = false;
                  ns->force_newmap = true;
                }
          }
}

int get_data_size (faceidx idx, int faceset = 0)
	CODE:
        facedata *d = face_data (idx, faceset);
        if (!d) XSRETURN_UNDEF;
        RETVAL = d->data.size ();
	OUTPUT:
        RETVAL

SV *get_chksum (faceidx idx, int faceset = 0)
	CODE:
        facedata *d = face_data (idx, faceset);
        if (!d) XSRETURN_UNDEF;
        RETVAL = newSVpvn ((char *)d->chksum, d->chksum_len);
	OUTPUT:
        RETVAL

SV *get_data (faceidx idx, int faceset = 0)
	CODE:
        facedata *d = face_data (idx, faceset);
        if (!d) XSRETURN_UNDEF;
        RETVAL = newSVpvn (d->data.data (), d->data.length ());
	OUTPUT:
        RETVAL

void invalidate (faceidx idx)
	CODE:
        for_all_clients (ns)
          {
            ns->faces_sent [idx] = false;
            ns->force_newmap = true;
          }

void invalidate_all ()
	CODE:
        for_all_clients (ns)
          {
            ns->faces_sent.reset ();
            ns->force_newmap = true;
          }

MODULE = cf        PACKAGE = cf::anim	PREFIX = anim_

#INCLUDE: $PERL $srcdir/genacc faceset $srcdir/../include/anim.h |

animidx anim_find (utf8_string name)
	CODE:
        RETVAL = animation::find (name).number;
        OUTPUT: RETVAL

animidx set (utf8_string name, SV *frames, int facings = 1)
	CODE:
{
  	if (!SvROK (frames) && SvTYPE (SvRV (frames)) != SVt_PVAV)
          croak ("frames must be an arrayref");

        AV *av = (AV *)SvRV (frames);

  	animation *anim = &animation::find (name);
        if (anim->number)
          {
            anim->resize (av_len (av) + 1);
            anim->facings = facings;
          }
        else
          anim = &animation::create (name, av_len (av) + 1, facings);

        for (int i = 0; i < anim->num_animations; ++i)
          anim->faces [i] = face_find (cfSvPVutf8_nolen (*av_fetch (av, i, 1)));
}
	OUTPUT: RETVAL

void invalidate_all ()
	CODE:
        for_all_clients (ns)
          ns->anims_sent.reset ();

MODULE = cf        PACKAGE = cf::object::freezer

INCLUDE: $PERL $srcdir/genacc object_freezer $srcdir/../include/cfperl.h |

SV *
new (char *klass)
	CODE:
        RETVAL = newSVptr (new object_freezer, gv_stashpv ("cf::object::freezer", 1));
	OUTPUT: RETVAL

void
DESTROY (SV *sv)
	CODE:
        object_freezer *self;
        sv_to (sv, self);
        delete self;

MODULE = cf        PACKAGE = cf::object::thawer

INCLUDE: $PERL $srcdir/genacc object_thawer $srcdir/../include/freezethaw.h |

bool
errors_are_fatal (bool fatal)
	CODE:
        RETVAL = object_thawer::errors_are_fatal;
        object_thawer::errors_are_fatal = fatal;
	OUTPUT:
        RETVAL

SV *
new_from_file (char *klass, octet_string path)
	CODE:
        object_thawer *f = new object_thawer (path);
        if (!*f)
          {
            delete f;
            XSRETURN_UNDEF;
          }
        RETVAL = newSVptr (f, gv_stashpv ("cf::object::thawer", 1));
	OUTPUT: RETVAL

void
DESTROY (SV *sv)
	CODE:
        object_thawer *self;
        sv_to (sv, self);
        delete self;

void
extract_tags (object_thawer *self)
	PPCODE:
        while (self->kw != KW_EOF)
          {
            PUTBACK;
            coroapi::cede_to_tick ();
            SPAGAIN;

            if (self->kw == KW_tag)
              XPUSHs (sv_2mortal (newSVpv_utf8 (self->get_str ())));

            self->skip ();
          }

