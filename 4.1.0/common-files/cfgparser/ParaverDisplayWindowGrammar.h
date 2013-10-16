#ifndef DOMAIN_PARAVERDISPLAYWINDOWGRAMMAR_H
#define DOMAIN_PARAVERDISPLAYWINDOWGRAMMAR_H


#include <string>
using namespace std;
#include <boost/spirit/core.hpp>
#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/spirit/actor/increment_actor.hpp>
#include <boost/bind.hpp>

#include <boost/spirit/phoenix/primitives.hpp>
#include <boost/spirit/phoenix/operators.hpp>
using namespace boost;
using namespace boost::spirit;
using namespace phoenix;


namespace domain {

template<typename Actions>
class ParaverDisplayWindowGrammar : public ::grammar<ParaverDisplayWindowGrammar<Actions> > {
  protected:
    Actions & actions;

    mutable string str1;

    mutable int int1;


  public:
    ParaverDisplayWindowGrammar(Actions & actions);

    template<typename ScannerT>
    class definition {
      protected:
        rule<ScannerT> anychar_less_eol_p;

        rule<ScannerT> display_window;

        rule<ScannerT> display_header;

        rule<ScannerT> window_vars;

        rule<ScannerT> window_name;

        rule<ScannerT> window_type;

        rule<ScannerT> window_id;

        rule<ScannerT> window_position_x;

        rule<ScannerT> window_position_y;

        rule<ScannerT> window_width;

        rule<ScannerT> window_height;

        rule<ScannerT> window_logical_filtered;

        rule<ScannerT> window_physical_filtered;

        rule<ScannerT> window_comm_lines_enabled;

        rule<ScannerT> window_send_icon_enabled;

        rule<ScannerT> window_recv_icon_enabled;

        rule<ScannerT> window_flags_enabled;

        rule<ScannerT> window_noncolor_mode;

        rule<ScannerT> window_factors;

        rule<ScannerT> window_operation;

        rule<ScannerT> window_identifiers;

        rule<ScannerT> window_level;

        rule<ScannerT> window_scale;

        rule<ScannerT> window_comm_fromto;

        rule<ScannerT> window_comm_tagsize;

        rule<ScannerT> window_comm_typeval;

        rule<ScannerT> window_minimum_y;

        rule<ScannerT> window_color_mode;

        rule<ScannerT> window_maximum_y;

        rule<ScannerT> window_units;

        rule<ScannerT> window_compute_y_max;

        rule<ScannerT> window_scale_relative;

        rule<ScannerT> window_selected_objects;

        rule<ScannerT> window_object;

        rule<ScannerT> window_begin_time;

        rule<ScannerT> window_end_time;

        rule<ScannerT> window_stop_time;

        rule<ScannerT> window_current_time;

        rule<ScannerT> window_begin_time_relative;

        rule<ScannerT> window_pos_to_disp;

        rule<ScannerT> window_pos_of_x_scale;

        rule<ScannerT> window_pos_of_y_scale;

        rule<ScannerT> window_number_of_row;

        rule<ScannerT> window_click_options;

        rule<ScannerT> window_click_info;

        rule<ScannerT> window_expanded;

        rule<ScannerT> window_open;

        rule<ScannerT> window_scrollbar;

        rule<ScannerT> window_drawmode;

        rule<ScannerT> window_drawmode_rows;

        rule<ScannerT> window_selected_functions;

        rule<ScannerT> window_compose_functions;

        rule<ScannerT> window_analyzer_executed;

        rule<ScannerT> window_semantic_module;

        rule<ScannerT> window_analyzer_info;

        rule<ScannerT> window_filter_module;


      public:
        definition(const ParaverDisplayWindowGrammar<Actions> & self);

        rule<ScannerT> const & start() const;

    };
    
};
template<typename Actions>
ParaverDisplayWindowGrammar<Actions>::ParaverDisplayWindowGrammar(Actions & actions):actions(actions) {
}

template<typename Actions>
template<typename ScannerT>
ParaverDisplayWindowGrammar<Actions>::definition<ScannerT>::definition(const ParaverDisplayWindowGrammar<Actions> & self) {
  Actions & actions = self.actions;
  
  display_window
     = *space_p >> !(display_header | window_vars) >> end_p
     ;
  
  display_header 
     =  ( +ch_p('#'))
        | ('<' >> str_p("NEW DISPLAYING WINDOW") >> +anychar_p)
     ;
  
  window_vars
     =
        (window_name | window_type | window_id | window_position_x | window_position_y
        | window_width | window_height | window_logical_filtered | window_physical_filtered
        | window_comm_lines_enabled | window_send_icon_enabled | window_recv_icon_enabled | window_flags_enabled
        | window_noncolor_mode |window_factors | window_operation | window_identifiers
        | window_units | window_compute_y_max | window_level | window_scale | window_comm_fromto | window_comm_tagsize | window_comm_typeval | window_minimum_y | window_color_mode
        | window_maximum_y |window_scale_relative | window_selected_objects | window_object | window_begin_time | window_end_time
        | window_stop_time | window_current_time | window_begin_time_relative
        | window_pos_to_disp | window_pos_of_x_scale | window_pos_of_y_scale | window_number_of_row
        | window_click_options | window_click_info | window_expanded | window_open | window_scrollbar
        | window_drawmode | window_drawmode_rows
        | window_selected_functions | window_compose_functions | window_analyzer_executed 
        | window_semantic_module | window_analyzer_info | window_filter_module)
     ;
  
  window_name 
     = str_p("window_name")
        >> (+anychar_p)[assign_a(self.str1)][bind(&Actions::set_name, ref(actions), ref(self.str1))]
     ;
  window_type
     = str_p("window_type") >> (
                                str_p("single")
                                | str_p("composed")[bind(&Actions::setComposed, ref(actions))]
                                )
     ;
  window_id = str_p("window_id") >> int_p;
  window_position_x = str_p("window_position_x") >> int_p;
  window_position_y = str_p("window_position_y") >> int_p;
  window_width = str_p("window_width") >> int_p;
  window_logical_filtered = str_p("window_logical_filtered") >> +anychar_p;
  window_physical_filtered = str_p("window_physical_filtered") >> +anychar_p;
  window_height = str_p("window_height") >> int_p;
  window_comm_lines_enabled = str_p("window_comm_lines_enabled") >> +alpha_p;
  window_send_icon_enabled = str_p("window_send_icon_enabled") >> +anychar_p;
  window_recv_icon_enabled = str_p("window_recv_icon_enabled") >> +anychar_p;
  window_flags_enabled = str_p("window_flags_enabled") >> +alpha_p;
  window_noncolor_mode = str_p("window_noncolor_mode") >> +anychar_p;
  window_factors = str_p("window_factors") >> +lexeme_d[real_p];
  window_operation = str_p("window_operation") >> +alpha_p;
  window_identifiers
     = str_p("window_identifiers")
     >> +lexeme_d[int_p][assign_a(self.int1)][bind(&Actions::addIdentifier, ref(actions), ref(self.int1))]
     ;
  window_units = str_p("window_units") >> +alpha_p;
  window_compute_y_max = str_p("window_compute_y_max");
  window_level = str_p("window_level") >> +alpha_p;
  window_scale = str_p("window_scale") >> +anychar_p;
  window_comm_fromto = str_p("window_comm_fromto") >> +anychar_p;
  window_comm_tagsize = str_p("window_comm_tagsize") >> +anychar_p;
  window_comm_typeval = str_p("window_comm_typeval") >> +anychar_p;
  window_minimum_y = str_p("window_minimum_y") >> real_p;
  window_color_mode = str_p("window_color_mode") >> +anychar_p;
  window_maximum_y = str_p("window_maximum_y") >> real_p;
  window_scale_relative = str_p("window_scale_relative") >> real_p;
  window_selected_objects = str_p("window_selected_objects") >> +anychar_p;
  window_object = str_p("window_object") >> +anychar_p; // Improve
  window_begin_time = str_p("window_begin_time") >> +anychar_p;
  window_end_time = str_p("window_end_time") >> +anychar_p;
  window_stop_time = str_p("window_stop_time") >> +anychar_p;
  window_current_time = str_p("window_current_time") >> +anychar_p;
  
  
  window_begin_time_relative = str_p("window_begin_time_relative") >> real_p;
  window_pos_to_disp = str_p("window_pos_to_disp") >> int_p;
  window_pos_of_x_scale = str_p("window_pos_of_x_scale") >> int_p;
  window_pos_of_y_scale = str_p("window_pos_of_y_scale") >> int_p;
  window_number_of_row = str_p("window_number_of_row") >> int_p;
  window_click_options = str_p("window_click_options") >> +int_p; // Attention "1 0 1 1 1 0" should use lexeme_d ?
  window_click_info = str_p("window_click_info") >> +lexeme_d[real_p];
  window_expanded = str_p("window_expanded") >> +alpha_p;
  window_open = str_p("window_open") >> +alpha_p;
  window_scrollbar = str_p("window_scrollbar") >> +anychar_p;
  window_drawmode = str_p("window_drawmode") >> lexeme_d[int_p];
  window_drawmode_rows = str_p("window_drawmode_rows") >> lexeme_d[int_p];
  window_selected_functions = str_p("window_selected_functions") >> +anychar_p; // Improve
  window_compose_functions = str_p("window_compose_functions") >> +anychar_p; // Improve
  window_semantic_module = str_p("window_semantic_module") >> +anychar_p;
  window_analyzer_executed = str_p("window_analyzer_executed") >> int_p;
  window_analyzer_info = str_p("window_analyzer_info") >> +real_p; // lexeme_d ?
  window_filter_module
     =
        str_p("window_filter_module") >>
        (str_p("evt_type") >> int_p >> +(lexeme_d[int_p][assign_a(self.int1)])[bind(&Actions::addDependencies, ref(actions), ref(self.int1))]
        |str_p("evt_value") >> int_p >> +(lexeme_d[int_p])
        |str_p("from_obj") >> int_p >> +(lexeme_d[int_p])
        |str_p("to_obj") >> int_p >> +(lexeme_d[int_p])
        )
     ;
  
  BOOST_SPIRIT_DEBUG_NODE(display_window);
  BOOST_SPIRIT_DEBUG_NODE(display_header);
  BOOST_SPIRIT_DEBUG_NODE(window_click_info);
  BOOST_SPIRIT_DEBUG_NODE(anychar_p);
  BOOST_SPIRIT_DEBUG_NODE(window_vars);
  BOOST_SPIRIT_DEBUG_NODE(window_type);
}

template<typename Actions>
template<typename ScannerT>
rule<ScannerT> const & ParaverDisplayWindowGrammar<Actions>::definition<ScannerT>::start() const {
  return display_window;
}


} // namespace domain
#endif
