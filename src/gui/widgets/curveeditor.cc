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

#include "curveeditor.hh"


//static const int curve_area_size = 300;
//static const int curve_area_margin = 5;


PF::CurveEditor::CurveEditor( OperationConfigGUI* dialog, std::string pname,
    CurveArea* ca, float _xmin, float _xmax, float _ymin, float _ymax,
    int width, int height, int border_size ):
  Gtk::HBox(),
  PF::PFWidget( dialog, pname ),
  xlabel( "in: " ),
  ylabel( "out: " ),
  xmin( _xmin ), xmax( _xmax ), ymin( _ymin ), ymax( _ymax ),
#ifdef GTKMM_2
  xadjustment( xmax, xmin, xmax, 1, 10, 0),
  yadjustment( ymax, ymin, ymax, 1, 10, 0),
  xspinButton(xadjustment),
  yspinButton(yadjustment),
#endif
  curve_area_width( width ),
  curve_area_height( height ),
  curve_area( ca ),
  grabbed_point( -1 ),
  button_pressed( false ),
  inhibit_value_changed( false )
{
#ifdef GTKMM_3
  xadjustment = Gtk::Adjustment::create( xmax, xmin, xmax, 1, 10, 0 );
  yadjustment = Gtk::Adjustment::create( ymax, ymin, ymax, 1, 10, 0 );
  xspinButton.set_adjustment( xadjustment );
  yspinButton.set_adjustment( yadjustment );
#endif
  curve_area->set_size_request( curve_area_width+border_size*2,
                              curve_area_height+border_size*2 );
  curve_area->set_border_size( border_size );

  xspinButton.set_digits( 1 );
  yspinButton.set_digits( 1 );

  curve_area_ebox.add( *curve_area );
  curve_area_ebox.add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );
#ifdef GTKMM_2
  curve_area_ebox.set_flags(Gtk::CAN_FOCUS);
#endif
#ifdef GTKMM_3
  curve_area_ebox.set_can_focus(TRUE);
#endif


  box.pack_start( curve_area_ebox );
  numentries_spacing.set_size_request(10,0);
  spin_buttons_box.pack_start( xlabel, Gtk::PACK_SHRINK );
  spin_buttons_box.pack_start( xspinButton, Gtk::PACK_SHRINK );
  spin_buttons_box.pack_start( numentries_spacing, Gtk::PACK_SHRINK );
  spin_buttons_box.pack_start( ylabel, Gtk::PACK_SHRINK );
  spin_buttons_box.pack_start( yspinButton, Gtk::PACK_SHRINK );
  box.pack_start( spin_buttons_box );

  pack_start( box, Gtk::PACK_SHRINK );

  curve_area->signal_event().connect( sigc::mem_fun(*this, &PF::CurveEditor::handle_curve_events) );
  curve_area_ebox.signal_key_press_event().connect( sigc::mem_fun(*this, &PF::CurveEditor::on_key_press_or_release_event) );
  curve_area_ebox.signal_key_release_event().connect( sigc::mem_fun(*this, &PF::CurveEditor::on_key_press_or_release_event) );
  // adjustment.signal_value_changed().
  //   connect(sigc::mem_fun(*this,
  // 			  &PFWidget::changed));

  xspinButton.signal_value_changed().
    connect(sigc::mem_fun(*this,
                          &CurveEditor::update_point));

  yspinButton.signal_value_changed().
    connect(sigc::mem_fun(*this,
                          &CurveEditor::update_point));

  show_all_children();
}


void PF::CurveEditor::update_point()
{
  //if( !curve_area->get_curve() ) return;
  if( inhibit_value_changed ) return;
  std::cout<<"PF::CurveEditor::update_point() called."<<std::endl;
  int ipt = curve_area->get_selected_point();
  if( (ipt >= 0) && (ipt < curve_area->get_curve().get_npoints()) ) {
#ifdef GTKMM_2
    float px = (xadjustment.get_value()-xmin)/(xmax-xmin);
    float py = (yadjustment.get_value()-ymin)/(ymax-ymin);
#endif
#ifdef GTKMM_3
    float px = (xadjustment->get_value()-xmin)/(xmax-xmin);
    float py = (yadjustment->get_value()-ymin)/(ymax-ymin);
#endif
    if( curve_area->get_curve().set_point( ipt, px, py ) ) {
      curve_area->get_curve().update_spline();
      curve_area->queue_draw();
      inhibit_value_changed = true;
#ifdef GTKMM_2
      xadjustment.set_value( px*(xmax-xmin)+xmin );
      yadjustment.set_value( py*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
      xadjustment->set_value( px*(xmax-xmin)+xmin );
      yadjustment->set_value( py*(ymax-ymin)+ymin );
#endif
      inhibit_value_changed = false;
      changed();
      get_prop()->modified();
    }
  }
}


void PF::CurveEditor::get_value()
{
  if( !get_prop() ) return;
  PF::Property<PF::SplineCurve>* prop = dynamic_cast< PF::Property<PF::SplineCurve>* >( get_prop() );
  if( !prop ) return;
  curve_area->set_curve( prop->get() );
  curve_area->set_selected_point( 0 );
  inhibit_value_changed = true;
#ifdef GTKMM_2
  xadjustment.set_value( prop->get().get_point(0).first*(xmax-xmin)+xmin );
  yadjustment.set_value( prop->get().get_point(0).second*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
  xadjustment->set_value( prop->get().get_point(0).first*(xmax-xmin)+xmin );
  yadjustment->set_value( prop->get().get_point(0).second*(ymax-ymin)+ymin );
#endif
  inhibit_value_changed = false;
  //std::cout<<"CurveEditor::get_value() called"<<std::endl;
}


void PF::CurveEditor::set_value()
{
  if( !get_prop() ) return;
  PF::Property<PF::SplineCurve>* prop = dynamic_cast< PF::Property<PF::SplineCurve>* >( get_prop() );
  if( !prop ) return;
  prop->set( curve_area->get_curve() );
}



void PF::CurveEditor::add_point( float xpt, float ycurve )
{
  SplineCurve& curve = curve_area->get_curve();
  int ipt = curve.add_point( xpt, ycurve );
  if( ipt >= 0 ) {
    curve_area->set_selected_point( ipt );
    grabbed_point = ipt;
    curve.update_spline();
    curve_area->queue_draw();
    inhibit_value_changed = true;
#ifdef GTKMM_2
    xadjustment.set_value( xpt*(xmax-xmin)+xmin );
    yadjustment.set_value( ycurve*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
    xadjustment->set_value( xpt*(xmax-xmin)+xmin );
    yadjustment->set_value( ycurve*(ymax-ymin)+ymin );
#endif
    inhibit_value_changed = false;
  }
}


bool PF::CurveEditor::handle_curve_events(GdkEvent* event)
{
  Gtk::Allocation allocation = curve_area->get_allocation();
  const int width = allocation.get_width();
  const int height = allocation.get_height();

  SplineCurve& curve = curve_area->get_curve();
  
  //if( !curve ) return false;
  //curve.lock();
  
  switch( event->type ) {

  case Gdk::BUTTON_PRESS: 
    {
#ifndef NDEBUG
      std::cout<<"PF::CurveArea::handle_events(): button pressed @ "
               <<event->button.x<<","<<event->button.y<<std::endl;
#endif
      button_pressed = true;
      
      curve_area_ebox.grab_focus();

      // Look for a point close to the mouse click
      double xpt = double(event->button.x-1)/(width-3);
      double ypt = double(height-event->button.y-1)/(height-3);
      //#ifndef NDEBUG
              std::cout<<"  xpt="<<xpt<<"  ypt="<<ypt<<std::endl;
      //#endif
      //std::vector< std::pair<float,float> > points = curve.get_points();
      //std::pair<float,float>* points = curve.get_points();
      bool found = false;
      int ipt = -1;
      for( unsigned int i = 0; i < curve.get_npoints(); i++ ) {
        double dx = fabs( xpt - curve.get_point(i).first);
        double dy = fabs( ypt - curve.get_point(i).second);
#ifndef NDEBUG
        std::cout<<"  point #"<<i<<"  dx="<<dx<<"  dy="<<dy<<std::endl;
#endif
        if( (dx<0.05) && (dy<0.05) ) {
          ipt = i;
          found = true;
          break;
        }
      }
      if( found ) {
        if( event->button.button == 1 ) {
          // We left-clicked on one existing point, so we grab it
          curve_area->set_selected_point( ipt );
          grabbed_point = ipt;
#ifndef NDEBUG
          std::cout<<"  point #"<<ipt<<" grabbed"<<std::endl;
#endif
          inhibit_value_changed = true;
#ifdef GTKMM_2
          xadjustment.set_value( curve.get_point(ipt).first*(xmax-xmin)+xmin );
          yadjustment.set_value( curve.get_point(ipt).second*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
          xadjustment->set_value( curve.get_point(ipt).first*(xmax-xmin)+xmin );
          yadjustment->set_value( curve.get_point(ipt).second*(ymax-ymin)+ymin );
#endif
          inhibit_value_changed = false;
          curve_area->queue_draw();
        } else if( event->button.button == 3 ) {
          curve.remove_point( ipt );
          curve.update_spline();
          curve_area->set_selected_point( 0 );
          curve_area->queue_draw();
          inhibit_value_changed = true;
#ifdef GTKMM_2
          xadjustment.set_value( curve.get_points()[0].first*(xmax-xmin)+xmin );
          yadjustment.set_value( curve.get_points()[0].second*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
          xadjustment->set_value( curve.get_points()[0].first*(xmax-xmin)+xmin );
          yadjustment->set_value( curve.get_points()[0].second*(ymax-ymin)+ymin );
#endif
          inhibit_value_changed = false;
          changed();
        }
      } else {
        if( event->button.button == 1 ) {
          // The click was far from any existing point, let's see if
          // it is on the curve and if we have to add one more point
          double ycurve = curve.get_value( xpt );
          double dy = fabs( ypt - ycurve);
          if( dy<0.05 ) {
            add_point( xpt, ycurve );
          }
        }
      }
      break;
    }
  case Gdk::BUTTON_RELEASE: 
    {
      if( (event->button.button==1) && (grabbed_point>=0) ) {
        changed();
        get_prop()->modified();
      }
#ifndef NDEBUG
      std::cout<<"Grabbed point cleared"<<std::endl;
#endif
      grabbed_point = -1;
    }
  case (Gdk::MOTION_NOTIFY) : 
    {
      //std::cout<<"grabbed point: "<<grabbed_point<<std::endl;
      if( /*!curve ||*/ (grabbed_point<0) ) break;

      int tx, ty;
      Gdk::ModifierType mod_type;
      if (event->motion.is_hint) {
        curve_area->get_window()->get_pointer (tx, ty, mod_type);
      }
      else {
        tx = int(event->button.x);
        ty = int(event->button.y);
        mod_type = (Gdk::ModifierType)event->button.state;
      }

      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      float px = double(tx-1)/(width-3);
      float py = double(height-ty-1)/(height-3);
      // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      if( curve.set_point( grabbed_point, px, py ) ) {
        curve.update_spline();
        curve_area->queue_draw();
        inhibit_value_changed = true;
#ifdef GTKMM_2
        xadjustment.set_value( px*(xmax-xmin)+xmin );
        yadjustment.set_value( py*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
        xadjustment->set_value( px*(xmax-xmin)+xmin );
        yadjustment->set_value( py*(ymax-ymin)+ymin );
#endif
        inhibit_value_changed = false;
      }
      break;
    }
  default:
    break;
  }
  //curve.unlock();

  return false;
}



bool PF::CurveEditor::on_key_press_or_release_event(GdkEventKey* event)
{
  if( (curve_area->get_selected_point()>=0) && event->type == GDK_KEY_PRESS &&
      (event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK | GDK_MOD1_MASK)) == 0 ) {

    SplineCurve& curve = curve_area->get_curve();

    std::pair<float,float> pt = curve.get_point( curve_area->get_selected_point() );
    float delta = 0.01;

    if( (event->keyval == GDK_KEY_Up) ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      pt.second += delta;
    }
    if( event->keyval == GDK_KEY_Down ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      pt.second -= delta;
    }
    if( event->keyval == GDK_KEY_Left ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      pt.first -= delta;
    }
    if( event->keyval == GDK_KEY_Right ) {
      std::cout<<"Pressed "<<event->keyval<<" key"<<std::endl;
      pt.first += delta;
    }

    float px = pt.first, py = pt.second;
    if( curve.set_point( curve_area->get_selected_point(), px, py ) ) {
      curve.update_spline();
      curve_area->queue_draw();
      inhibit_value_changed = true;
#ifdef GTKMM_2
      xadjustment.set_value( px*(xmax-xmin)+xmin );
      yadjustment.set_value( py*(ymax-ymin)+ymin );
#endif
#ifdef GTKMM_3
      xadjustment->set_value( px*(xmax-xmin)+xmin );
      yadjustment->set_value( py*(ymax-ymin)+ymin );
#endif
      inhibit_value_changed = false;
      changed();
      get_prop()->modified();
    }
  }
  return true;
}


PF::CurveArea::CurveArea(): border_size( 0 ), selected_point( -1 )
{
  this->add_events(Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::BUTTON_MOTION_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK );
  //this->add_events(Gdk::BUTTON_RELEASE_MASK);
  //this->add_events(Gdk::BUTTON_MOTION_MASK);
#ifdef GTKMM_2
  set_flags(Gtk::CAN_FOCUS);
#endif
#ifdef GTKMM_3
  set_can_focus(TRUE);
#endif
}


#ifdef GTKMM_2
bool PF::CurveArea::on_expose_event(GdkEventExpose* event)
{
  // This is where we draw on the window
  Glib::RefPtr<Gdk::Window> window = get_window();
  if( !window )
    return true;

  Gtk::Allocation allocation = get_allocation();
  const int width = allocation.get_width() - border_size*2;
  const int height = allocation.get_height() - border_size*2;
  const int x0 = border_size;
  const int y0 = border_size;

  Cairo::RefPtr<Cairo::Context> cr = window->create_cairo_context();

  if( false && event ) {
    // clip to the area indicated by the expose event so that we only
    // redraw the portion of the window that needs to be redrawn
    cr->rectangle(event->area.x, event->area.y,
                  event->area.width, event->area.height);
    cr->clip();
  }
#endif
#ifdef GTKMM_3
  bool PF::CurveArea::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width() - border_size*2;
    const int height = allocation.get_height() - border_size*2;
    const int x0 = border_size;
    const int y0 = border_size;
#endif

    cr->save();
    cr->set_source_rgba(0.2, 0.2, 0.2, 1.0);
    cr->paint();
    cr->restore();

    // Draw outer rectangle
    cr->set_antialias( Cairo::ANTIALIAS_GRAY );
    cr->set_source_rgb( 0.9, 0.9, 0.9 );
    cr->set_line_width( 0.5 );
    cr->rectangle( double(0.5+x0-1), double(0.5+y0-1), double(width+1), double(height+1) );
    cr->stroke ();


    draw_background( cr );

    // Draw curve
    if( /*curve*/ true ) {
      curve.lock();
      cr->set_source_rgb( 0.9, 0.9, 0.9 );
      curve.update_spline();
      std::vector< std::pair<float,float> > vec;
      //std::cout<<"PF::CurveArea::on_expose_event(): width="<<width<<"  height="<<height<<std::endl;
      for( int i = 0; i < width; i++ ) {
        float fi = i;
        vec.push_back( std::make_pair( fi/(width-1), (float)0 ) );
      }
      curve.get_values( vec );

      // draw curve
      cr->set_source_rgb( 0.9, 0.9, 0.9 );
      cr->move_to( double(vec[0].first)*width+x0, double(1.0f-vec[0].second)*height+y0 );
      for (unsigned int i=1; i<vec.size(); i++) {
        cr->line_to( double(vec[i].first)*width+x0, double(1.0f-vec[i].second)*height+y0 );
      }
      cr->stroke ();

      //std::vector< std::pair<float,float> > points = curve.get_points();
      for( unsigned int i = 0; i < curve.get_npoints(); i++ ) {
        double x = double(curve.get_point(i).first)*width+x0;
        double y = double(1.0f-curve.get_point(i).second)*height+y0;
        cr->set_source_rgb( 0.9, 0.9, 0.9 );
        cr->arc (x, y, 3.5, 0, 2*M_PI);
        cr->fill ();
        if( i == selected_point ) {
          cr->set_source_rgb( 0.9, 0.0, 0.0 );
          cr->arc (x, y, 2., 0, 2*M_PI);
          cr->fill ();
        }
      }
      curve.unlock();
    }
  
    return true;
  }



  void PF::CurveArea::draw_background(const Cairo::RefPtr<Cairo::Context>& cr)
  {
    Gtk::Allocation allocation = get_allocation();
    const int width = allocation.get_width() - border_size*2;
    const int height = allocation.get_height() - border_size*2;
    const int x0 = border_size;
    const int y0 = border_size;

    // Draw grid
    cr->set_source_rgb( 0.9, 0.9, 0.9 );
    std::vector<double> ds (2);
    ds[0] = 4;
    ds[1] = 4;
    cr->set_dash (ds, 0);
    cr->move_to( double(0.5+x0+width/4), double(y0) );
    cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(0.5+x0+width/2), double(y0) );
    cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(0.5+x0+width*3/4), double(y0) );
    cr->rel_line_to (double(0), double(height) );
    cr->move_to( double(x0), double(0.5+y0+height/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height/2) );
    cr->rel_line_to (double(width), double(0) );
    cr->move_to( double(x0), double(0.5+y0+height*3/4) );
    cr->rel_line_to (double(width), double(0) );
    cr->stroke ();
    cr->unset_dash ();

    ds[0] = 2;
    ds[1] = 4;
    cr->set_source_rgb( 0.5, 0.5, 0.5 );
    cr->set_dash (ds, 0);
    for( int i = 1; i <= 7; i += 2 ) {
      cr->move_to( double(0.5+x0+width*i/8), double(y0) );
      cr->rel_line_to (double(0), double(height) );
      cr->move_to( double(x0), double(0.5+y0+height*i/8) );
      cr->rel_line_to (double(width), double(0) );
    }
    cr->stroke ();
    cr->unset_dash ();
  }



