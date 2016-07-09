/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file create_image.hpp
 *
 * @brief LaTeX functions for printing circuits
 */

#ifndef CREATE_IMAGE_HPP
#define CREATE_IMAGE_HPP

#include <iostream>
#include <fstream>
#include <string>

#include <core/circuit.hpp>

namespace revkit
{

  /**
   * @brief Generic class for the create_image function (Template Design Pattern)
   *
   * This class is never used directly. Instead use other derived classes
   * as create_latex_settings or derive an image creation class on your own.
   *
   * All pure virtual methods have to be implemented and the methods
   * draw_before, draw_in_between and draw_after are supposed to be overridden
   * for special use cases.
   */
  class create_image_settings
  {
  public:
    /** @cond false */
    virtual ~create_image_settings() {}
    /** @endcond */

    /**
     * @brief Begin with drawing
     *
     * This method is called when it is started to paint
     * the image.
     *
     * @param os Output stream of the create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_begin( std::ostream& os ) const = 0;

    /**
     * @brief Draws a circuit line
     *
     * This method draws a circuit line.
     *
     * @param os Output stream of the create_image function
     * @param x1 Horizontal starting position of the line
     * @param x2 Horizontal ending position of the line
     * @param y  Vertical position of the line
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_line( std::ostream& os, float x1, float x2, float y ) const = 0;

    /**
     * @brief Draws an input name
     *
     * This methods draws an input name at the starting position of a line
     *
     * @param os Output stream of the create_image function
     * @param x  Horizontal starting position of the ascending line
     * @param y  Vertical position of the ascending line
     * @param text Name of the input
     * @param is_constant Specifies whether the input is a constant input
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const = 0;

    /**
     * @brief Draws an output name
     *
     * This methods draws an output name at the starting position of a line
     *
     * @param os Output stream of the create_image function
     * @param x  Horizontal ending position of the ascending line
     * @param y  Vertical position of the ascending line
     * @param text Name of the output
     * @param is_garbage Specifies whether the output is a garbage output
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const = 0;

    /**
     * @brief Draws a control
     *
     * This methods draws a control
     *
     * @param os Output stream of the create_image function
     * @param x  X-coordinate of the center of the control
     * @param y  Y-coordinate of the center of the control
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_control( std::ostream& os, float x, float y ) const = 0;

    /**
     * @brief Draws a control
     *
     * This methods draws a control
     *
     * @param os Output stream of the create_image function
     * @param x  X-coordinate of the center of the targets
     * @param ys Y-coordinates of the centers of the targets
     * @param target_tag Type of the target
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const = 0;

    /**
     * @brief Draws the frame around Peres Gates
     *
     * This method draws the frame around Peres gates.
     *
     * @param os Output stream of the create_image function
     * @param x1 Left X-coordinate
     * @param y1 Bottom Y-coordinate
     * @param x2 Right X-coordinate
     * @param y2 Top Y-coordinate
     * 
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const = 0;

    /**
     * @brief Draws a gate line
     *
     * Draws a line from the top most element to the bottom most element
     *
     * @param os Output stream of the create_image function
     * @param x  X-coordinate of the line
     * @param y1 Top Y-coordinate of the line
     * @param y2 Bottom Y-coordinate of the line
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const = 0;

    /**
     * @brief Stops ending the image
     *
     * This methods is called after finishing painting the
     * image.
     *
     * @param os Output stream of the create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_end( std::ostream& os ) const = 0;

  public:
    /**
     * @brief User callback for user-defined drawing before the circuit is painted
     *
     * @param os Output stream of the create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_before( std::ostream& os ) const;

    /**
     * @brief User callback for user-defined drawing after the lines but before the gates are painted
     *
     * @param os Output stream of the create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_in_between( std::ostream& os ) const;

    /**
     * @brief User callback for user-defined drawing after the circuit was painted
     *
     * @param os Output stream of the create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    virtual void draw_after( std::ostream& os ) const;
        
    /**
     * @brief With of the image
     *
     * Set by create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    float width;

    /**
     * @brief Height of the image
     *
     * Set by create_image function
     *
     * @author RevKit
     * @since  1.0
     */
    float height;
    
    /**
     * @brief Width of a gate
     *
     * No default value, can be different for
     * different create_image_settings implementations.
     *
     * @author RevKit
     * @since  1.0
     */
    float elem_width;

    /**
     * @brief Height of a gate
     *
     * No default value, can be different for
     * different create_image_settings implementations.
     *
     * @author RevKit
     * @since  1.0
     */
    float elem_height;

    /**
     * @brief Width of a line
     *
     * No default value, can be different for
     * different create_image_settings implementations.
     *
     * @author RevKit
     * @since  1.0
     */
    float line_width;

    /**
     * @brief Radius of a control element
     *
     * No default value, can be different for
     * different create_image_settings implementations.
     *
     * @author RevKit
     * @since  1.0
     */
    float control_radius;

    /**
     * @brief Radius of a target element
     *
     * No default value, can be different for
     * different create_image_settings implementations.
     *
     * @author RevKit
     * @since  1.0
     */
    float target_radius;    

    /**
     * @brief Text written before the circuit is printed
     *
     * Default value is empty
     *
     * @author RevKit
     * @since  1.0
     */
    std::string draw_before_text;

    /**
     * @brief Text written after the lines but before the gates are printed
     *
     * Default value is empty
     *
     * @author RevKit
     * @since  1.0
     */
    std::string draw_in_between_text;

    /**
     * @brief Text written after the circuit is printed
     *
     * Default value is empty
     *
     * @author RevKit
     * @since  1.0
     */
    std::string draw_after_text;
  };
    
  /**
   * @brief Implementation of create_image_settings for generating LaTeX code using PsTricks
   *
   * For drawing the circuit the PsTricks package is used. For more information
   * check http://www.pstricks.de
   *
   * @author RevKit
   * @since  1.0
   */
  class create_pstricks_settings : public create_image_settings
  {
  public:
    /**
     * @brief Default constructor
     *
     * @author RevKit
     * @since  1.0
     */
    create_pstricks_settings();

    /** @cond false */
    virtual ~create_pstricks_settings() {}
    /** @endcond */

    virtual void draw_begin( std::ostream& os ) const;
    virtual void draw_line( std::ostream& os, float x1, float x2, float y ) const;
    virtual void draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const;
    virtual void draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const;

    virtual void draw_control( std::ostream& os, float x, float y ) const;
    virtual void draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const;
    virtual void draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const;
    virtual void draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const;
    virtual void draw_end( std::ostream& os ) const;

  public:
    /**
     * @brief Specifies whether inputs and outputs should be put in $ ... $ math mode
     *
     * Default value is \b false.
     *
     * @author RevKit
     * @since  1.0
     */
    bool math_emph;
  };

  /**
   * @brief Implementation of create_image_settings for generating LaTeX code using TikZ
   *
   * For drawing the circuit the TikZ package is used. For more information
   * check http://sourceforge.net/projects/pgf/
   *
   * @author RevKit
   * @since  1.0
   */
  class create_tikz_settings : public create_image_settings
  {
  public:
    /**
     * @brief Default constructor
     *
     * @author RevKit
     * @since  1.0
     */
    create_tikz_settings();

    /** @cond false */
    virtual ~create_tikz_settings() {}
    /** @endcond */

    virtual void draw_begin( std::ostream& os ) const;
    virtual void draw_line( std::ostream& os, float x1, float x2, float y ) const;
    virtual void draw_input( std::ostream& os, float x, float y, const std::string& text, bool is_constant ) const;
    virtual void draw_output( std::ostream& os, float x, float y, const std::string& text, bool is_garbage ) const;

    virtual void draw_control( std::ostream& os, float x, float y ) const;
    virtual void draw_targets( std::ostream& os, float x, const std::vector<float>& ys, const boost::any& target_tag ) const;
    virtual void draw_peres_frame( std::ostream& os, float x1, float y1, float x2, float y2 ) const;
    virtual void draw_gate_line( std::ostream& os, float x, float y1, float y2 ) const;
    virtual void draw_end( std::ostream& os ) const;

  public:
    /**
     * @brief Specifies whether inputs and outputs should be put in $ ... $ math mode
     *
     * Default value is \b false.
     *
     * @author RevKit
     * @since  1.0
     */
    bool math_emph;
  };

  /**
   * @brief Create image from circuit \p circ and write it to \p os
   *
   * This function creates an image from a circuit.
   * Thereby, the third parameter is a struct which derives from create_image_settings,
   * for example create_pstricks_settings of create_tikz_settings for creating LaTeX
   * pictures based on PsTricks or TikZ, respectively.
   *
   * In order to write to an unsupported format, derive a class from create_image_settings
   * and re-implement the abstract methods.
   *
   * @param os Output stream to write the image to
   * @param circ Circuit
   * @param settings The settings for painting which provides as well the painting methods
   *
   * @author RevKit
   * @since  1.0
   */
  void create_image( std::ostream& os, const circuit& circ, create_image_settings& settings );

  /**
   * @brief Wrapper for create_image to save to a file
   *
   * This is a wrapper for create_image(std::ostream& os, const circuit&, create_image_settings&)
   * which creates a ostream object for a file buffer.
   *
   * @param filename File-name
   * @param circ Circuit
   * @param settings The settings for painting which provides as well the painting methods
   *
   * @author RevKit
   * @since  1.0
   */
  void create_image( const std::string& filename, const circuit& circ, create_image_settings& settings );
}

#endif /* CREATE_IMAGE_HPP */
