#ifndef __MOLLERDETECTORCONSTRUCTION_HH
#define __MOLLERDETECTORCONSTRUCTION_HH

#include "globals.hh"
#include "G4VUserDetectorConstruction.hh"
#include "remollGlobalField.hh"
#include "remollQuadField.hh"
#include "remollDipoleField.hh"
#include <vector>

#define __FIELDARRAYSIZE 10

class remollIO;
class G4Tubs;
class G4LogicalVolume;
class G4Material;
class G4VPhysicalVolume;
class G4VSensitiveDetector;
class G4FieldManager;
class remollQuadField;
class remollDipoleField;

class remollDetectorConstruction : public G4VUserDetectorConstruction
{
	public:
		remollDetectorConstruction();
		virtual ~remollDetectorConstruction();

	public:
		G4VPhysicalVolume* Construct();

		void CreateGlobalMagneticField();

		void SetDetectorGeomFile(const G4String&);

		remollGlobalField* GetGlobalField(){ return fGlobalField; }

                int getIndex(){return index;}
                void SetIndex(int i){index = i;}
                remollQuadField* getQuadByIndex(int i);
                remollDipoleField* getDipoleByIndex(int i);

		void SetIO( remollIO *io ){ fIO = io; }

		G4double meas_loc_x,meas_loc_y,meas_loc_z;
	private:

		G4FieldManager*		fGlobalFieldManager;
		remollGlobalField* 	fGlobalField;
		remollQuadField*        fQuadFields[__FIELDARRAYSIZE];
                remollDipoleField*      fDipoleFields[__FIELDARRAYSIZE];
                int                     index;
                G4String 		fDetFileName;
                G4Material*  g4vacuum;

		remollIO *fIO;

		//void DumpGeometricalTree(G4VPhysicalVolume* aVolume,G4int depth=0);
		//G4int UpdateCopyNo(G4VPhysicalVolume* aVolume,G4int index=0);

};
#endif
