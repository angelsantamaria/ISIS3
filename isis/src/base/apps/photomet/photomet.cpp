#include "Isis.h"
#include <string>
#include "Camera.h"
#include "ProcessByLine.h"
#include "SpecialPixel.h"
#include "Photometry.h"
#include "Pvl.h"
#include "Cube.h"

#include "PvlGroup.h"
#include "iException.h"

#include "Angle.h"

using namespace std;
using namespace Isis;

// Global variables
Camera *cam;
Cube *icube;
Photometry *pho;
double maxema;
double maxinc;
bool useDem;

void photomet(Buffer &in, Buffer &out);

void IsisMain() {
  // We will be processing by line
  ProcessByLine p;

  // Set up the input cube and get camera information
  icube = p.SetInputCube("FROM");
  cam = icube->Camera();

  // Create the output cube
  p.SetOutputCube("TO");

  // Set up the user interface
  UserInterface &ui = Application::GetUserInterface();
  
  // Get the name of the parameter file
  Pvl par(ui.GetFilename("PHOPAR"));
  
  // Set value for maximum emission/incidence angles chosen by user
  maxema = ui.GetDouble("MAXEMISSION");
  maxinc = ui.GetDouble("MAXINCIDENCE");
  
  // determine how photometric angles should be calculated
  useDem = ui.GetBoolean("USEDEM");
  
  // Get the BandBin Center from the image
  PvlGroup pvlg = icube->GetGroup("BandBin");
  double wl;
  if(pvlg.HasKeyword("Center")) {
    PvlKeyword &wavelength = pvlg.FindKeyword("Center");
    wl = wavelength[0];
  }
  else {
    wl = 1.0;
  }

  // Create the photometry object and set the wavelength
  PvlGroup &algo = par.FindObject("NormalizationModel").FindGroup("Algorithm", Pvl::Traverse);
  if(!algo.HasKeyword("Wl")) {
    algo.AddKeyword(Isis::PvlKeyword("Wl", wl));
  }
  pho = new Photometry(par);
  pho->SetPhotomWl(wl);

  // Start the processing
  p.StartProcess(photomet);
  p.EndProcess();
}

/**
 * Perform photometric correction
 *
 * @param in Buffer containing input DN values
 * @param out Buffer containing output DN values
 * @author Janet Barrett
 * @internal
 *   @history 2009-01-08 Jeannie Walldren - Modified to set off
 *            target pixels to null.  Added check for new maxinc
 *            and maxema parameters.
 */
void photomet(Buffer &in, Buffer &out) {

  double dempha=0., deminc=0., demema=0., mult=0., base=0.;
  double ellipsoidpha=0., ellipsoidinc=0., ellipsoidema=0.;
  
  for (int i = 0; i < in.size(); i++) {
  
    // if special pixel, copy to output
    if(!IsValidPixel(in[i])) {
      out[i] = in[i];
    }
    
    // if off the target, set to null
    else if(!cam->SetImage(in.Sample(i), in.Line(i))) {
      out[i] = NULL8;
    }
    
    // otherwise, compute angle values
    else {
      
      // calculate photometric angles
      ellipsoidpha = cam->PhaseAngle();
      ellipsoidinc = cam->IncidenceAngle();
      ellipsoidema = cam->EmissionAngle();
      bool success = true;
      if (useDem) {
        Angle phase, incidence, emission;
        cam->LocalPhotometricAngles(phase, incidence, emission, success);
        if (success) {
          dempha = phase.GetDegrees();
          deminc = incidence.GetDegrees();
          demema = emission.GetDegrees();
        }
      } else {
        dempha = ellipsoidpha;
        deminc = ellipsoidinc;
        demema = ellipsoidema;
      }

      // if invalid angles, set to null
      if(!success) {
        out[i] = NULL8;
      }
      else if(deminc >= 90.0 || demema >= 90.0) {
        out[i] = NULL8;
      }
      // if angles greater than max allowed by user, set to null
      else if(deminc > maxinc || demema > maxema) {
        out[i] = NULL8;
      }
      // otherwise, do photometric correction
      else {
        pho->Compute(ellipsoidpha, ellipsoidinc, ellipsoidema, deminc, demema, in[i], out[i], mult, base);
      }
    }
  }
}
