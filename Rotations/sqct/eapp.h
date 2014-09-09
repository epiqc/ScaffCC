//     Copyright (c) 2013 Jeff Heckey jheckey(at)ece(dot)ucsb(dot)edu
//     This file uses SQCT, Copyright (c) 2012 Vadym Kliuchnikov, Dmitri Maslov, Michele Mosca;
//     SQCT is distributed under LGPL v3
//

#include "sk.h"
#include "netgenerator.h"

#include <fstream>
#include <sstream>

#ifndef EAPP_H
#define EAPP_H

/// \brief Options for epsilon net generation
struct enetOptions
{
    /// \brief If one element specified -- upper bound for sde of epsilon net to be
    /// generated. If two elements specified -- interval of sde to be generated.
    std::vector<int> epsilon_net_layers;
};

////////////////////////////////////////////////////////////////////

struct enetApplication
{
    enetApplication( const enetOptions& options ) :
        m_options(options)
    {}

    /// \brief Checks if all layers generated on initial state are available
    void check_initial()
    {
        initial_ok = true;
        if( m_layers[0] == 0 )
            initial_ok = false;

        for( int i = 2; (i < initial_end) && initial_ok ; ++i )
            if( m_layers[i] == 0 ) initial_ok = false;
    }

    /// \brief Collects information about parts of epsilon net available
    void check_files()
    {
         m_layers.resize(100,0);
         // collect information about available layers
         for( int i = 0; i < 100; ++i )
         {
             std::string name = netGenerator::fileName(i);
             std::ifstream ifs(name);
             if( !ifs )
                 m_layers[i] = 0;
             else
                 m_layers[i] = 1;
         }
    }

    /// \brief Generates all layers with \f$ sde(|\cdot|^2) \f$ in the interval [st,end)
    void generate( int st, int end )
    {
        if( ! initial_ok && st < initial_end )
            m_ng.generateInitial();

        for( int i = std::max(initial_end,st) ; i < end; ++i )
        {
            if( m_layers[i] == 0 )
            {
                //std::cout << "\tGenerating Layer " << i << " of " << end << std::endl;
                epsilonnet base_net;
                base_net.loadFromFile( netGenerator::fileName(i-1).c_str() );
                std::unique_ptr<epsilonnet> res( netGenerator::generate( base_net ) );
                res->saveToFile( netGenerator::fileName(i).c_str() );
            }
        }
    }

    /// \brief Do the work
    void process()
    {
        check_files();
        check_initial();

        int sz  = m_options.epsilon_net_layers.size();
        int b = m_options.epsilon_net_layers[0];
        switch( sz )
        {
        case 1:
            generate(0,b + 1);
            break;
        case 2:
            generate(b,m_options.epsilon_net_layers[1] + 1);
            break;
        default:
            std::cerr << "Wrong number of parameters. See help." << std::endl;
        }
    }

    void print_error_message()
    {
        std::cout << "Epsilon net files a not available, use --epsilon-net option" << std::endl
             << "to generate them." << std::endl;
    }

    bool check()
    {
        int st  = 0;
        int end = m_options.epsilon_net_layers[0] + 1;
        check_files();
        check_initial();

        if( ! initial_ok && st < initial_end )
        {
            //print_error_message();
            return false;
        }

        for( int i = std::max(initial_end,st) ; i < end; ++i )
        {
            if( m_layers[i] == 0 )
            {
                print_error_message();
                return false;
            }
        }
        return true;
    }

    const enetOptions& m_options;       ///< Application options
    std::vector<int> m_layers;          ///< Ones for available layers, zeros for not availible layers
    static const int initial_end;		///< Maximal layer generated on initial state
    bool initial_ok;                    ///< If all initial layers are availible
    netGenerator m_ng;                  ///< Epsilon net generator
};

const int enetApplication::initial_end = 21;

#endif // EAPP_H

