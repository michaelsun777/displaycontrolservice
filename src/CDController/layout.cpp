/*
   Copyright 2018 Alexander Courtis

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/
#include "layout.h"

#include "xrandrrutil.h"
#include "xrdbutil.h"
#include "xutil.h"
#include "calculations.h"
#include <iostream>
#include <unistd.h>
#include <vector>

#include "../cspdlog.h"

using namespace std;

namespace CDController {

int layout(const Settings& settings) 
{
    // optional wait
    if (settings.wait) {
        if (!settings.quiet) {
            cout << "Waiting " << settings.wait << " seconds..." << endl;
        }
        sleep(settings.wait);
    }

    // discover monitors
    const Monitors monitors = Monitors();


    vector<shared_ptr<Mode>> vSupportModes;
    // discover outputs
    const list<shared_ptr<Output>> currentOutputs = discoverOutputs(vSupportModes);
    if (currentOutputs.empty()) {
        throw runtime_error("no outputs found");
    }

    // output verbose information
    if (!settings.quiet || settings.info) {
        cout << renderUserInfo(currentOutputs) << "\n\n";
        cout << "laptop lid ";
        if (monitors.laptopLidClosed) {
            cout << "closed";
        }
        else {
            cout << "open or not present";
        }
        cout << "\n";
    }

    // current info is all output, we're done
    if (settings.info) {
        return EXIT_SUCCESS;
    }

    // order the outputs if the user wishes
    const list<shared_ptr<Output>> outputs = orderOutputs(currentOutputs, settings.order);

    

    // activate ouputs and determine primary
    const shared_ptr<Output> primary = activateOutputs(outputs, settings.primary, monitors);

    
    // arrange mirrored or left to right
    if (settings.mirror) {
        mirrorOutputs(outputs);
    }
    else 
    {
        bool bIsFoundMode = false;
        shared_ptr<Mode> pMode;
        if (vSupportModes.size() > 0 && settings.width > 0 && settings.height > 0)
        {
            for (size_t i = 0; i < vSupportModes.size(); i++)
            {
                if (vSupportModes[i]->width == settings.width && vSupportModes[i]->height == settings.height)
                {

                    pMode = vSupportModes[i];
                    bIsFoundMode = true;
                    cout << "found mode :" << vSupportModes[i]->width << "x" << vSupportModes[i]->height << "@" << vSupportModes[i]->refresh << "Hz\n";
                    break;
                }
            }
        }

        if (bIsFoundMode)
        {
            //ltrOutputs_ex(outputs, pMode);
            ltrOutputs_dst(outputs, pMode,settings.outputsPos);
        }
        else
        {
            ltrOutputs(outputs);
        }

        
    }

    // determine DPI from the primary
    string dpiExplaination;
    long dpi = calculateDpi(primary, &dpiExplaination);
    if (!settings.quiet) {
        cout << "\n" << dpiExplaination << "\n";
    }

    // user overrides DPI
    if (settings.dpi) {
        dpi = settings.dpi;
        cout << "overriding with provided DPI " << to_string(dpi) << "\n";
    }

    // user overrides refresh rate
    long rate = 0;
    if (settings.rate) {
        rate = settings.rate;
        cout << "overriding with provided refresh rate " << to_string(rate) << "\n";
    }

    // render desired commands
    const string xrandrCmd = renderXrandrCmd(outputs, primary, dpi, rate);
    const string xrdbCmd = renderXrdbCmd(dpi);
    if (!settings.quiet || settings.noop) {
        cout << "\n" << xrandrCmd << "\n\n" << xrdbCmd << "\n";
        XINFO("CDController layout xrandrCmd:{}\nxrdbCmd:{}\n",xrandrCmd,xrdbCmd);
    }

    //return 0;
    // execute
    if (!settings.noop) {
        // xrandr
        int status = system(xrandrCmd.c_str());

        if (WIFEXITED(status))
        {
            printf("normal termination,exit status = %d\n", WEXITSTATUS(status));
        }

        else if (WIFSIGNALED(status))
        {
            printf("abnormal termination,signal number =%d%s\n", WTERMSIG(status),
#ifdef WCOREDUMP
                   WCOREDUMP(status) ? "core file generated" : "");
#else
                   "");
#endif
        }

        if (status != 0) 
        {
            return status;
        }


        // xrdb
        status = system(xrdbCmd.c_str());
        if (status != 0) {
            return status;
        }

        // update root window's cursor
        resetRootCursor();
    }
    return EXIT_SUCCESS;
}


int layout_check(const std::shared_ptr<CDController::Settings> & settings) 
{
    vector<shared_ptr<Mode>> vSupportModes;
    // discover outputs
    const list<shared_ptr<Output>> currentOutputs = discoverOutputs(vSupportModes);
    if (currentOutputs.empty()) {
        throw runtime_error("no outputs found");
    }

    // order the outputs if the user wishes
    const list<shared_ptr<Output>> outputs = orderOutputs(currentOutputs, settings->order);
    // discover monitors
    const Monitors monitors = Monitors();    

    // activate ouputs and determine primary
    const shared_ptr<Output> primary = activateOutputs(outputs, settings->primary, monitors);

    bool bRet = checkOutputs_dst(outputs,settings->outputsPos,settings->width,settings->height);
    return bRet?EXIT_SUCCESS:EXIT_FAILURE;
}

}