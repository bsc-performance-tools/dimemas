#ifndef DOMAIN_PARAVERANALYZERGRAMMAR_H
#define DOMAIN_PARAVERANALYZERGRAMMAR_H


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
class ParaverAnalyzerGrammar : public ::grammar<ParaverAnalyzerGrammar<Actions> > {
  protected:
    Actions & actions;

    mutable int int1;


  public:
    ParaverAnalyzerGrammar(Actions & actions);

    template<typename ScannerT>
    class definition {
      protected:
        rule<ScannerT> anychar_less_eol_p;

        rule<ScannerT> anychar_less_starter_p;

        rule<ScannerT> analyzer2d;

        rule<ScannerT> analyzer2d_header;

        rule<ScannerT> analyzer2d_vars;

        rule<ScannerT> analyzer2d_name;

        rule<ScannerT> analyzer2d_x;

        rule<ScannerT> analyzer2d_y;

        rule<ScannerT> analyzer2d_width;

        rule<ScannerT> analyzer2d_height;

        rule<ScannerT> analyzer2d_controlwindow;

        rule<ScannerT> analyzer2d_accumulator;

        rule<ScannerT> analyzer2d_statistic;

        rule<ScannerT> analyzer2d_datawindow;

        rule<ScannerT> analyzer2d_horizvert;

        rule<ScannerT> analyzer2d_color;

        rule<ScannerT> analyzer2d_semanticcolor;

        rule<ScannerT> analyzer2d_calculateall;

        rule<ScannerT> analyzer2d_zoom;

        rule<ScannerT> analyzer2d_textmode;

        rule<ScannerT> analyzer2d_expanded;

        rule<ScannerT> analyzer2d_hidecols;

        rule<ScannerT> analyzer2d_accumulatebycontrolwindow;

        rule<ScannerT> analyzer2d_sortcols;

        rule<ScannerT> analyzer2d_sortcriteria;

        rule<ScannerT> analyzer2d_parameters;

        rule<ScannerT> analyzer2d_analysislimits;

        rule<ScannerT> analyzer2d_showwindows;

        rule<ScannerT> analyzer2d_computeyscale;

        rule<ScannerT> analyzer2d_minimum;

        rule<ScannerT> analyzer2d_maximum;

        rule<ScannerT> analyzer2d_delta;

        rule<ScannerT> analyzer2d_computegradient;

        rule<ScannerT> analyzer2d_minimumgradient;

        rule<ScannerT> analyzer2d_maximumgradient;

        rule<ScannerT> analyzer2d_3D_ControlWindow;

        rule<ScannerT> analyzer2d_3D_Minimum;

        rule<ScannerT> analyzer2d_3D_Maximum;

        rule<ScannerT> analyzer2d_3D_Delta;

        rule<ScannerT> analyzer2d_3D_FixedValue;


      public:
        definition(const ParaverAnalyzerGrammar<Actions> & self);

        rule<ScannerT> const & start() const;

    };
    
};
template<typename Actions>
ParaverAnalyzerGrammar<Actions>::ParaverAnalyzerGrammar(Actions & actions):actions(actions) {
}

template<typename Actions>
template<typename ScannerT>
ParaverAnalyzerGrammar<Actions>::definition<ScannerT>::definition(const ParaverAnalyzerGrammar<Actions> & self) {
  Actions & actions = self.actions;
  
  analyzer2d
     = *space_p >> !(analyzer2d_header | analyzer2d_vars) >> end_p
     ;
  
  
  analyzer2d_header = ch_p('<') >> str_p("NEW ANALYZER2D") >> '>';
  
  analyzer2d_vars
     =
        (analyzer2d_name | analyzer2d_x | analyzer2d_y | analyzer2d_width | analyzer2d_height | analyzer2d_controlwindow
        | analyzer2d_accumulator | analyzer2d_statistic | analyzer2d_datawindow | analyzer2d_horizvert | analyzer2d_color
        | analyzer2d_semanticcolor | analyzer2d_calculateall
        | analyzer2d_zoom | analyzer2d_textmode | analyzer2d_expanded | analyzer2d_hidecols | analyzer2d_accumulatebycontrolwindow 
        | analyzer2d_sortcols | analyzer2d_sortcriteria | analyzer2d_parameters | analyzer2d_analysislimits
        | analyzer2d_showwindows |analyzer2d_computeyscale | analyzer2d_minimum | analyzer2d_maximum | analyzer2d_delta
        | analyzer2d_computegradient | analyzer2d_minimumgradient | analyzer2d_maximumgradient | analyzer2d_3D_ControlWindow
        | analyzer2d_3D_Minimum | analyzer2d_3D_Maximum | analyzer2d_3D_Delta | analyzer2d_3D_FixedValue)
     ;
  
  analyzer2d_name = str_p("Analyzer2D.Name:") >> +anychar_less_eol_p;
  analyzer2d_x = str_p("Analyzer2D.X:") >> int_p;
  analyzer2d_y = str_p("Analyzer2D.Y:") >> int_p;
  analyzer2d_width = str_p("Analyzer2D.Width:") >> int_p;
  analyzer2d_height = str_p("Analyzer2D.Height:") >> int_p;
  analyzer2d_controlwindow
     =
        str_p("Analyzer2D.ControlWindow:")
        >> int_p[assign_a(self.int1)][bind(&Actions::addControlWindow, ref(actions), ref(self.int1))]
     ;
  analyzer2d_accumulator = str_p("Analyzer2D.Accumulator:") >> +alpha_p;
  analyzer2d_statistic = str_p("Analyzer2D.Statistic:") >> +anychar_less_eol_p;
  analyzer2d_datawindow
     =
        str_p("Analyzer2D.DataWindow:")
        >> int_p[assign_a(self.int1)][bind(&Actions::addDataWindow, ref(actions), ref(self.int1))]
     ;
  analyzer2d_horizvert = str_p("Analyzer2D.HorizVert:") >> +alpha_p;
  analyzer2d_color = str_p("Analyzer2D.Color:") >> +anychar_p;
  analyzer2d_semanticcolor = str_p("Analyzer2D.SemanticColor:") >> +anychar_p;
  analyzer2d_calculateall = str_p("Analyzer2D.CalculateAll:") >> +alpha_p;
  analyzer2d_zoom = str_p("Analyzer2D.Zoom:") >> +anychar_less_eol_p;
  analyzer2d_textmode = str_p("Analyzer2D.TextMode:") >> +alpha_p;
  analyzer2d_expanded = str_p("Analyzer2D.Expanded:") >> +alpha_p;
  analyzer2d_hidecols = str_p("Analyzer2D.HideCols:") >> +alpha_p;
  analyzer2d_accumulatebycontrolwindow = str_p("Analyzer2D.AccumulateByControlWindow:") >> +anychar_p;
  analyzer2d_sortcols = str_p("Analyzer2D.SortCols:") >> +anychar_p;
  analyzer2d_sortcriteria = str_p("Analyzer2D.SortCriteria:") >> +anychar_p;
  analyzer2d_parameters = str_p("Analyzer2D.Parameters:") >> +anychar_less_eol_p; // Improve
  analyzer2d_analysislimits = str_p("Analyzer2D.AnalysisLimits:") >> +alpha_p;
  analyzer2d_showwindows = str_p("Analyzer2D.ShowWindows:") >> +alpha_p;
  analyzer2d_computeyscale = str_p("Analyzer2D.ComputeYScale:") >> +alpha_p;
  analyzer2d_minimum = str_p("Analyzer2D.Minimum:") >> real_p;
  analyzer2d_maximum = str_p("Analyzer2D.Maximum:") >> real_p;
  analyzer2d_delta = str_p("Analyzer2D.Delta:") >> real_p;
  analyzer2d_computegradient = str_p("Analyzer2D.ComputeGradient:") >> +alpha_p;
  analyzer2d_minimumgradient = str_p("Analyzer2D.MinimumGradient:") >> real_p;
  analyzer2d_maximumgradient = str_p("Analyzer2D.MaximumGradient:") >> real_p;
  
  analyzer2d_3D_ControlWindow = str_p("Analyzer2D.3D_ControlWindow:") >> int_p;
  analyzer2d_3D_Minimum = str_p("Analyzer2D.3D_Minimum:") >> real_p;
  analyzer2d_3D_Maximum = str_p("Analyzer2D.3D_Maximum:") >> real_p;
  analyzer2d_3D_Delta = str_p("Analyzer2D.3D_Delta:") >> real_p;
  analyzer2d_3D_FixedValue = str_p("Analyzer2D.3D_FixedValue:") >> int_p;
  
  anychar_less_eol_p = anychar_p - eol_p;
  anychar_less_starter_p = anychar_p - '<' - '#';
  
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_header);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_vars);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_accumulator);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_statistic);
  BOOST_SPIRIT_DEBUG_NODE(analyzer2d_datawindow);
  BOOST_SPIRIT_DEBUG_NODE(anychar_less_eol_p);
}

template<typename Actions>
template<typename ScannerT>
rule<ScannerT> const & ParaverAnalyzerGrammar<Actions>::definition<ScannerT>::start() const {
  return analyzer2d;
}


} // namespace domain
#endif
