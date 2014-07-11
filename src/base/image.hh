/* 
   
 */

/*

    Copyright (C) 2014 Ferrero Andrea

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.


 */

/*

    These files are distributed with PhotoFlow - http://aferrero2707.github.io/PhotoFlow/

 */

#ifndef PF_IMAGE_HH
#define PF_IMAGE_HH

#include <stdlib.h>
#include <sigc++/sigc++.h>

#include "layermanager.hh"
#include "view.hh"


namespace PF
{


  class Image: public sigc::trackable
  {
    LayerManager layer_manager;
    std::vector<View*> views;

    // Flag indicating whether the update should be preformed asynchronously
    bool async;

    // Flag indicating whether the views have to be re-built
    bool modified;

    // Flag indicating whether there is a re-building ongoing
    bool rebuilding;

		// Current "pfi" file name associated to this image
		std::string file_name;

    GMutex* rebuild_mutex;
    GCond* rebuild_done;

    ProcessorBase* convert2srgb;
    ProcessorBase* convert_format;

    void remove_from_inputs( PF::Layer* layer );
    void remove_from_inputs( PF::Layer* layer, std::list<Layer*>& list );
    void remove_layer( PF::Layer* layer, std::list<Layer*>& list );

    void update_async();

  public:
    Image();

    ~Image();

    //sigc::signal<void> signal_modified;

    LayerManager& get_layer_manager() { return layer_manager; }

    void remove_layer( PF::Layer* layer );

    void add_view( VipsBandFormat fmt, int level )
    {
      views.push_back( new View( this, fmt, level ) );
    }

    unsigned int get_nviews() { return views.size(); }

    View* get_view(unsigned int n) 
    {
      if( n >= views.size() ) return NULL;
      return(views[n]);
    }

    bool is_async() { return async; }
    void set_async( bool flag ) { async = flag; }

    bool is_modified() { return modified; }
    void set_modified( bool flag ) { modified = flag; }

    bool is_rebuilding() { return rebuilding; }
    void set_rebuilding( bool flag ) { rebuilding = flag; }

    //Glib::Threads::Mutex& get_rebuild_mutex() { return rebuild_mutex; }

    void lock() { g_mutex_lock( rebuild_mutex); }
    void unlock() { g_mutex_unlock( rebuild_mutex); }
    void rebuild_done_signal() { g_cond_signal( rebuild_done ); }

    void update( VipsRect* area=NULL );
    void update_all() { update( NULL ); }
    void do_update( VipsRect* area );

    bool open( std::string filename );

		std::string get_filename() { return file_name; }
    bool save( std::string filename );
    bool export_merged( std::string filename );
  };

  gint image_rebuild_callback( gpointer data );

}


#endif /*VIPS_PARITHMETIC_H*/


