#include "remollDetectorConstruction.hh"
#include "G4SystemOfUnits.hh"
#include "G4PhysicalConstants.hh"
#include "remollBeamTarget.hh"
#include "remollIO.hh"

#include "G4TransportationManager.hh"
#include "G4FieldManager.hh"
#include "remollGenericDetector.hh"
#include "G4SDManager.hh"

#include "G4NistManager.hh"
#include "G4Material.hh"
#include "G4MaterialTable.hh"
#include "G4Element.hh"
#include "G4ElementTable.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4Box.hh"
#include "G4Tubs.hh"
#include "G4VPhysicalVolume.hh"
#include "G4LogicalVolume.hh"
#include "G4RotationMatrix.hh"
#include "G4ThreeVector.hh"
#include "G4Transform3D.hh"
#include "G4PVPlacement.hh"
#include "G4OpticalSurface.hh"
#include "G4VisAttributes.hh"
#include "remollglobs.hh"
#include "remollDipoleField.hh"
#include "remollQuadField.hh"

remollDetectorConstruction::remollDetectorConstruction()
{
	meas_loc_x = 0.*cm;
	meas_loc_y = 50.*cm;
	meas_loc_z = 100.*cm;
	

	fIO = NULL;

	CreateGlobalMagneticField();
	

}

remollDetectorConstruction::~remollDetectorConstruction(){;}

G4VPhysicalVolume* remollDetectorConstruction::Construct()
{
        
	//return ConstructDipole();
	return ConstructChicane();

}

G4VPhysicalVolume* remollDetectorConstruction::ConstructDipole()
{
	remollBeamTarget *beamtarg = remollBeamTarget::GetBeamTarget();
	beamtarg->Reset();
	/////
	// Material Definitions
	
	G4double a, z, density;
	G4int nelements;

	G4Element* H = new G4Element("Hydrogen", "H", z=1 , a = 1.00794*g/mole);
	G4Material* TargetGas = new G4Material("TargetGas", density=5e-9*g/cm3, nelements=1);
	TargetGas->AddElement(H, 1);

	G4Element* N = new G4Element("Nitrogen", "N", z=7, a=14.01*g/mole);
	G4Element* O = new G4Element("Oxygen", "O", z=8, a=16.00*g/mole);
	G4Material* Air = new G4Material("Air", density=1.29*mg/cm3, nelements=2);
	Air->AddElement(N, 70.*perCent);
	Air->AddElement(O, 30.*perCent);

	G4Element* Al = new G4Element("Aluminum", "Al", z=13, a=26.98*g/mole);
	G4Material* Aluminum = new G4Material("TargetWall", density=2.703*g/cm3, nelements=1);
	Aluminum->AddElement(Al, 1);


	/////
	// Grab the detector manager
	
	G4SDManager* SDman = G4SDManager::GetSDMpointer();


	///////////////////////////////
	///// ----- Volumes ----- /////

	G4double world_r, world_z;
	G4double starting_angle, spanning_angle;
	G4double targ_r, targ_z;
	G4double det_r, det_z;
	G4double container_inner_r, container_outer_r, container_z;
	G4double meas_x, meas_y, meas_z;
	G4double readplane_r, readplane_z_pos;
	
	world_r = 10*m;
	world_z = 2000.*cm;
	starting_angle = 0.*deg;
	spanning_angle = 360.*deg;
	//det_r = 250.*cm;
        det_r = 500.*cm;
	det_z = 2000.*cm;
	targ_r = 1.*cm;
	targ_z = 10.*cm;
	container_inner_r = 1.0*targ_r;
	container_outer_r = 50.0*cm;;
	container_z = 1.0*targ_z;
	meas_x = 3*cm;
	meas_y = 3*meas_x;
	meas_z = meas_x/3;
	readplane_r = 4*m;
	readplane_z_pos = 40.0*cm;//40.0*cm;

	/////
	// World
	
	G4Tubs* world_cyl 
		= new G4Tubs("World",0.,world_r,world_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* world_log
		= new G4LogicalVolume(world_cyl,Air,"World",0,0,0);
	
	world_log->SetVisAttributes(G4VisAttributes::Invisible);
	

	G4VPhysicalVolume* world_phys
		= new G4PVPlacement(NULL,G4ThreeVector(0.,0.,0.),world_log,"World",0,false,0);
	beamtarg->SetMotherVolume(world_phys);
	
	/////
	// Detector system intialization
	
	G4Tubs* det_cyl 
		= new G4Tubs("Detector",0.,det_r,det_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* det_log
		= new G4LogicalVolume(det_cyl,Air,"Detector_log",0,0,0);

	det_log->SetVisAttributes(G4VisAttributes::Invisible);

	/////
	// Target cell building and sensetization

	G4Tubs* targ_cyl 
		= new G4Tubs("Target",0.,targ_r,targ_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* targ_log
		= new G4LogicalVolume(targ_cyl,TargetGas,"Target",0,0,0);

	remollGenericDetector* targSD 
		= new remollGenericDetector("TargetSD",1);



	//SDman->AddNewDetector(targSD);
	//targ_log->SetSensitiveDetector(targSD);

	G4RotationMatrix* rotTarg
		= new G4RotationMatrix;
	rotTarg->rotateZ(0.*deg);

	G4VPhysicalVolume* targ_phys
		= new G4PVPlacement(rotTarg,G4ThreeVector(0.,0.,0.),targ_log,"Target",
			det_log,false,0);

	beamtarg->AddVolume(targ_phys);

	
	G4Tubs* mgf_cyl 
		= new G4Tubs("mgf",0.,targ_r,targ_r,starting_angle,spanning_angle);
	
	G4LogicalVolume* mgf_log
		= new G4LogicalVolume(mgf_cyl,Air,"mgf",0,0,0);

	G4VPhysicalVolume* mgf_phys
		= new G4PVPlacement(rotTarg,G4ThreeVector(0.0, -0.5*m, 220.4*cm),mgf_log,"mgf",
			det_log,false,0);


	/////
	// External container blocking some mollers from exiting the target cell
	// therefore, hitting det == 2 should be a veto parameter, made of Al for now
	// for no particular reason
	
	G4Tubs* container_cyl 
		= new G4Tubs("Container",container_inner_r,container_outer_r,container_z,starting_angle,spanning_angle);

	G4LogicalVolume* container_log 
		= new G4LogicalVolume(container_cyl,Aluminum,"Container",0,0,0);

	remollGenericDetector* containerSD
		= new remollGenericDetector("ContainerSD",gBoreDetNum);

	SDman->AddNewDetector(containerSD);
	container_log->SetSensitiveDetector(containerSD);
	
	G4RotationMatrix* rotContainer
		= new G4RotationMatrix;
	rotContainer->rotateZ(0.*deg);

	G4VPhysicalVolume* container_phys
		= new G4PVPlacement(rotContainer,G4ThreeVector(0.,0.,0.),container_log,"Container",
			det_log,false,0);
/*
	
	/////
	// Measurement device (just sensitive regions used to find coincident particles)
	
	G4Box* up_meas_box
		= new G4Box("UpMeasurer",meas_x,meas_y,meas_z);

	G4LogicalVolume* up_meas_log
		= new G4LogicalVolume(up_meas_box,Aluminum,"UpMeasurer",0,0,0);

	remollGenericDetector* up_measSD
		= new remollGenericDetector("UpMeasurerSD",3);

	//SDman->AddNewDetector(up_measSD);
	//up_meas_log->SetSensitiveDetector(up_measSD);

	G4RotationMatrix* rotUM
		= new G4RotationMatrix;
	rotUM->rotateZ(0.*deg);

	G4VPhysicalVolume* up_meas_phys
		= new G4PVPlacement(rotUM,G4ThreeVector(meas_loc_x,meas_loc_y,meas_loc_z),up_meas_log,"UpMeasurer",
			det_log,false,0);

	// Down measurer
	
	G4Box* down_meas_box
		= new G4Box("DownMeasurer",meas_x,meas_y,meas_z);

	G4LogicalVolume* down_meas_log
		= new G4LogicalVolume(down_meas_box,Aluminum,"DownMeasurer",0,0,0);

	remollGenericDetector* down_measSD
		= new remollGenericDetector("DownMeasurerSD",4);

	//SDman->AddNewDetector(down_measSD);
	//down_meas_log->SetSensitiveDetector(down_measSD);

	G4RotationMatrix* rotDM
		= new G4RotationMatrix;
	rotDM->rotateZ(180.*deg);

	G4VPhysicalVolume* down_meas_phys
		= new G4PVPlacement(rotDM,G4ThreeVector(meas_loc_x,-meas_loc_y,meas_loc_z),down_meas_log,"DownMeasurer",
			det_log,false,0);
*/

	// Make a flat plane detector and register with SDman.

	G4Tubs* readplane_cyl 
		= new G4Tubs("ReadPlane",0.,readplane_r,0.1*mm,starting_angle,spanning_angle);
	
	G4LogicalVolume* readplane5_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane",0,0,0);

	remollGenericDetector* readplaneSD5
		= new remollGenericDetector("ReadPlaneSD_5",5);

	SDman->AddNewDetector(readplaneSD5);
	readplane5_log->SetSensitiveDetector(readplaneSD5);


	G4LogicalVolume* readplane6_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane6",0,0,0);
	remollGenericDetector* readplaneSD6 
		= new remollGenericDetector("ReadPlaneSD_6",6);
	SDman->AddNewDetector(readplaneSD6);
	readplane6_log->SetSensitiveDetector(readplaneSD6);

	G4LogicalVolume* readplane7_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane7",0,0,0);
	remollGenericDetector* readplaneSD7 
		= new remollGenericDetector("ReadPlaneSD_7",7);
	SDman->AddNewDetector(readplaneSD7);
	readplane7_log->SetSensitiveDetector(readplaneSD7);

	G4LogicalVolume* readplane8_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane8",0,0,0);
	remollGenericDetector* readplaneSD8 
		= new remollGenericDetector("ReadPlaneSD_8",8);
	SDman->AddNewDetector(readplaneSD8);
	readplane8_log->SetSensitiveDetector(readplaneSD8);

	G4LogicalVolume* readplane9_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane9",0,0,0);
	remollGenericDetector* readplaneSD9 
		= new remollGenericDetector("ReadPlaneSD_9",9);
	SDman->AddNewDetector(readplaneSD9);
	readplane9_log->SetSensitiveDetector(readplaneSD9);

        ////////////////////////////////////////////////////////////
	G4RotationMatrix* rotRead
		= new G4RotationMatrix;
	rotRead->rotateZ(0.*deg);

	G4VPhysicalVolume* readplane_phys5
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,readplane_z_pos),readplane5_log,"ReadPlane5",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys6
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,readplane_z_pos+30*cm),readplane6_log,"ReadPlane6",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys7
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,100*cm),readplane7_log,"ReadPlane7",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys8
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,110*cm),readplane8_log,"ReadPlane8",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys9
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,110*cm+1*m/sqrt(2.0)),readplane9_log,"ReadPlane9",
			det_log,false,0);

	/////
	// Place the detector volume

	G4RotationMatrix* detrot
		= new G4RotationMatrix;
	detrot->rotateZ(0.*deg);

	G4VPhysicalVolume* det_phys
		= new G4PVPlacement(detrot,G4ThreeVector(0.,0.,0.),det_log,"detector_phys",
			world_log,false,0);


	///// --- End Volumes --- /////
	///////////////////////////////
	
	return world_phys;
}

G4VPhysicalVolume* remollDetectorConstruction::ConstructChicane()
{
	remollBeamTarget *beamtarg = remollBeamTarget::GetBeamTarget();
	beamtarg->Reset();
	/////
	// Material Definitions
	
	G4double a, z, density;
	G4int nelements;

	G4Element* H = new G4Element("Hydrogen", "H", z=1 , a = 1.00794*g/mole);
	G4Material* TargetGas = new G4Material("TargetGas", density=5e-9*g/cm3, nelements=1);
	TargetGas->AddElement(H, 1);

	G4Element* N = new G4Element("Nitrogen", "N", z=7, a=14.01*g/mole);
	G4Element* O = new G4Element("Oxygen", "O", z=8, a=16.00*g/mole);
	G4Material* Air = new G4Material("Air", density=1.29*mg/cm3, nelements=2);
	Air->AddElement(N, 70.*perCent);
	Air->AddElement(O, 30.*perCent);

	G4Element* Al = new G4Element("Aluminum", "Al", z=13, a=26.98*g/mole);
	G4Material* Aluminum = new G4Material("TargetWall", density=2.703*g/cm3, nelements=1);
	Aluminum->AddElement(Al, 1);


	/////
	// Grab the detector manager
	
	G4SDManager* SDman = G4SDManager::GetSDMpointer();
	G4NistManager* matman = G4NistManager::Instance();
	g4vacuum = matman->FindOrBuildMaterial("G4_Galactic");


	///////////////////////////////
	///// ----- Volumes ----- /////

	G4double world_r, world_z;
	G4double starting_angle, spanning_angle;
	G4double targ_r, targ_z;
	G4double det_r, det_z;
	G4double container_inner_r, container_outer_r, container_z;
	G4double meas_x, meas_y, meas_z;
	G4double readplane_r, readplane_z_pos, readplane_r1, box_r1;
	
	//world_r = 10*m;
        world_r = 15*m;
	world_z = 2000.*cm;
	starting_angle = 0.*deg;
	spanning_angle = 360.*deg;
	//det_r = 250.*cm;
	//det_r = 500.*cm;
        det_r = 1000.*cm;
	det_z = 2000.*cm;
	targ_r = 1.*cm;
	targ_z = 10.*cm;
	container_inner_r = 1.0*targ_r;
	container_outer_r = 50.0*cm;;
	container_z = 1.0*targ_z;
	meas_x = 3*cm;
	meas_y = 3*meas_x;
	meas_z = meas_x/3;
	readplane_r = 6*m;
        readplane_r1 = 5*m;
        box_r1 = 10*cm;
	readplane_z_pos = 40.0*cm;//40.0*cm;
        
         
	/////
	// World
	
	G4Tubs* world_cyl 
		= new G4Tubs("World",0.,world_r,world_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* world_log
	  //= new G4LogicalVolume(world_cyl,Air,"World",0,0,0);
	  = new G4LogicalVolume(world_cyl,g4vacuum,"World",0,0,0);
	
	world_log->SetVisAttributes(G4VisAttributes::Invisible);
	

	G4VPhysicalVolume* world_phys
		= new G4PVPlacement(NULL,G4ThreeVector(0.,0.,0.),world_log,"World",0,false,0);
	beamtarg->SetMotherVolume(world_phys);
	
	/////
	// Detector system intialization
	
	G4Tubs* det_cyl 
		= new G4Tubs("Detector",0.,det_r,det_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* det_log
	 //	= new G4LogicalVolume(det_cyl,Air,"Detector_log",0,0,0);
                = new G4LogicalVolume(det_cyl,g4vacuum,"Detector_log",0,0,0);

	det_log->SetVisAttributes(G4VisAttributes::Invisible);

	/////
	// Target cell building and sensetization

	G4Tubs* targ_cyl 
		= new G4Tubs("Target",0.,targ_r,targ_z,starting_angle,spanning_angle);
	
	G4LogicalVolume* targ_log
		= new G4LogicalVolume(targ_cyl,TargetGas,"Target",0,0,0);

	remollGenericDetector* targSD 
		= new remollGenericDetector("TargetSD",1);



	//SDman->AddNewDetector(targSD);
	//targ_log->SetSensitiveDetector(targSD);

	G4RotationMatrix* rotTarg
		= new G4RotationMatrix;
	rotTarg->rotateZ(0.*deg);

	G4VPhysicalVolume* targ_phys
		= new G4PVPlacement(rotTarg,G4ThreeVector(0.,0.,0.),targ_log,"Target",
			det_log,false,0);

	beamtarg->AddVolume(targ_phys);

	/////
	// External container blocking some mollers from exiting the target cell
	// therefore, hitting det == 2 should be a veto parameter, made of Al for now
	// for no particular reason
	
	G4Tubs* container_cyl 
		= new G4Tubs("Container",container_inner_r,container_outer_r,container_z,starting_angle,spanning_angle);

	G4LogicalVolume* container_log 
		= new G4LogicalVolume(container_cyl,Aluminum,"Container",0,0,0);

	remollGenericDetector* containerSD
		= new remollGenericDetector("ContainerSD",gBoreDetNum);

	SDman->AddNewDetector(containerSD);
	container_log->SetSensitiveDetector(containerSD);
	
	G4RotationMatrix* rotContainer
		= new G4RotationMatrix;
	rotContainer->rotateZ(0.*deg);

	G4VPhysicalVolume* container_phys
		= new G4PVPlacement(rotContainer,G4ThreeVector(0.,0.,0.),container_log,"Container",
			det_log,false,0);	
         
        ///////////////////////////////////////////
	G4Tubs* readplane_cyl 
		= new G4Tubs("ReadPlane",0.,readplane_r,0.1*mm,starting_angle,spanning_angle);
	
	G4LogicalVolume* readplane5_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane",0,0,0);

	remollGenericDetector* readplaneSD5
		= new remollGenericDetector("ReadPlaneSD_5",5);

	SDman->AddNewDetector(readplaneSD5);
	readplane5_log->SetSensitiveDetector(readplaneSD5);


	G4LogicalVolume* readplane6_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane6",0,0,0);
	remollGenericDetector* readplaneSD6 
		= new remollGenericDetector("ReadPlaneSD_6",6);
	SDman->AddNewDetector(readplaneSD6);
	readplane6_log->SetSensitiveDetector(readplaneSD6);

	G4LogicalVolume* readplane7_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane7",0,0,0);
	remollGenericDetector* readplaneSD7 
		= new remollGenericDetector("ReadPlaneSD_7",7);
	SDman->AddNewDetector(readplaneSD7);
	readplane7_log->SetSensitiveDetector(readplaneSD7);

	G4LogicalVolume* readplane8_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane8",0,0,0);
	remollGenericDetector* readplaneSD8 
		= new remollGenericDetector("ReadPlaneSD_8",8);
	SDman->AddNewDetector(readplaneSD8);
	readplane8_log->SetSensitiveDetector(readplaneSD8);

	G4LogicalVolume* readplane9_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane9",0,0,0);
	remollGenericDetector* readplaneSD9 
		= new remollGenericDetector("ReadPlaneSD_9",9);
	SDman->AddNewDetector(readplaneSD9);
	readplane9_log->SetSensitiveDetector(readplaneSD9);

	G4LogicalVolume* readplane10_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane10",0,0,0);
	remollGenericDetector* readplaneSD10
		= new remollGenericDetector("ReadPlaneSD_10",10);
	SDman->AddNewDetector(readplaneSD10);
	readplane10_log->SetSensitiveDetector(readplaneSD10);

	G4LogicalVolume* readplane11_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane11",0,0,0);
	remollGenericDetector* readplaneSD11 
		= new remollGenericDetector("ReadPlaneSD_11",11);
	SDman->AddNewDetector(readplaneSD11);
	readplane11_log->SetSensitiveDetector(readplaneSD11);

	G4LogicalVolume* readplane12_log
		= new G4LogicalVolume(readplane_cyl,Air,"ReadPlane12",0,0,0);
	remollGenericDetector* readplaneSD12 
		= new remollGenericDetector("ReadPlaneSD_12",12);
	SDman->AddNewDetector(readplaneSD12);
	readplane12_log->SetSensitiveDetector(readplaneSD12);

        ////////////////////////////////////////////////////////////
	G4RotationMatrix* rotRead
		= new G4RotationMatrix;
	rotRead->rotateZ(0.*deg);

	G4VPhysicalVolume* readplane_phys5
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,readplane_z_pos),readplane5_log,"ReadPlane5",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys6
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,readplane_z_pos+30*cm),readplane6_log,"ReadPlane6",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys7
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,87.5*cm),readplane7_log,"ReadPlane7",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys8
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,137.5*cm),readplane8_log,"ReadPlane8",
			det_log,false,0);

        G4VPhysicalVolume* readplane_phys9
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,157.5*cm),readplane9_log,"ReadPlane9",
			det_log,false,0);

	/*G4VPhysicalVolume* readplane_phys9
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,180*cm),readplane9_log,"ReadPlane9",
			det_log,false,0);*/

        G4VPhysicalVolume* readplane_phys10
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,217.5*cm),readplane10_log,"ReadPlane10",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys11
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,232.7871*cm),readplane11_log,"ReadPlane11",
			det_log,false,0);

	G4VPhysicalVolume* readplane_phys12
		= new G4PVPlacement(rotRead,G4ThreeVector(0.,0.,265*cm),readplane12_log,"ReadPlane12",
			det_log,false,0);


	/////
	// Place the detector volume

	G4RotationMatrix* detrot
		= new G4RotationMatrix;
	detrot->rotateZ(0.*deg);

	G4VPhysicalVolume* det_phys
		= new G4PVPlacement(detrot,G4ThreeVector(0.,0.,0.),det_log,"detector_phys",
			world_log,false,0);


	///// --- End Volumes --- /////
	///////////////////////////////
	
	return world_phys;
}


/*
G4int remollDetectorConstruction::UpdateCopyNo(G4VPhysicalVolume* aVolume,G4int index){
	aVolume->SetCopyNo(index);
	index++;
	for(int i=0;i<aVolume->GetLogicalVolume()->GetNoDaughters();i++){
		index = UpdateCopyNo(aVolume->GetLogicalVolume()->GetDaughter(i),index);
	}
	return index;
}


void remollDetectorConstruction::DumpGeometricalTree(G4VPhysicalVolume* aVolume,G4int depth)
{
	for(int isp=0;isp<depth;isp++)
		{ G4cout << "  "; }
	G4cout << aVolume->GetName() << "[" << aVolume->GetCopyNo() << "] "
		<< aVolume->GetLogicalVolume()->GetName() << " "
		<< aVolume->GetLogicalVolume()->GetNoDaughters() << " "
		<< aVolume->GetLogicalVolume()->GetMaterial()->GetName() << " "
		<< G4BestUnit(aVolume->GetLogicalVolume()->GetMass(true),"Mass");
	if(aVolume->GetLogicalVolume()->GetSensitiveDetector())
	{
		G4cout << " " << aVolume->GetLogicalVolume()->GetSensitiveDetector()
					->GetFullPathName();
	}
	G4cout << G4endl;
	for(int i=0;i<aVolume->GetLogicalVolume()->GetNoDaughters();i++)
	{ DumpGeometricalTree(aVolume->GetLogicalVolume()->GetDaughter(i),depth+1); }
}
*/
void remollDetectorConstruction::CreateGlobalMagneticField() {

	//CreateGlobalMagneticFieldDipole();
	CreateGlobalMagneticFieldChicane();
	return;
}

void remollDetectorConstruction::CreateGlobalMagneticFieldDipole(){

	fGlobalField = new remollGlobalField();
        
        //Create default quadrupole and dipole fields.        
        remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,55.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, -0.163*tesla);

        //Points above y = 0 are negative, below y = 0 are positive
        remollDipoleField *dipoleNeg = new remollDipoleField(G4ThreeVector(0.0, 2.0*m, 105.0*cm), G4ThreeVector(2.0*m, 2.0*m, 5.0*cm), -1.65*tesla);
        remollDipoleField *dipolePos = new remollDipoleField(G4ThreeVector(0.0, -2.0*m, 105.0*cm), G4ThreeVector(2.0*m, 2.0*m, 5.0*cm), 1.65*tesla);
        
        //Add new fields to their respective arrays
        fQuadFields[0] = quad;
        fDipoleFields[0] = dipoleNeg;
        fDipoleFields[1] = dipolePos;

        //Add new fields to be considered as part of the GlobalField
        fGlobalField -> AddNewField(quad);
        fGlobalField -> AddNewField(dipoleNeg);
        fGlobalField -> AddNewField(dipolePos);
	
        fGlobalFieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
	fGlobalFieldManager->SetDetectorField(fGlobalField);
	fGlobalFieldManager->CreateChordFinder(fGlobalField); 

	return;
}

void remollDetectorConstruction::CreateGlobalMagneticFieldChicane(){

	fGlobalField = new remollGlobalField();
        
        //Create default quadrupole and dipole fields.
	//remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,55.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, 0.163*tesla);//original design2 field focus in y defocus in x
        remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,55.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, -0.163*tesla);//original design2 field flipped for focus in x defucus in y
	//remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,55.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, -0.180*tesla);//original design2 field flipped for focus in x defucus in y
	//remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,55.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, 0.326*tesla);//field doubled
	//remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,50.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, 0.326*tesla);//field doubled and move away from dipole
	//remollQuadField *quad = new remollQuadField(G4ThreeVector(0.0,0.0,60.0*cm), G4ThreeVector(0.1*m, 0.1*m, 15.0*cm), 0.1*m, -0.15*tesla);//original design2 field flipped for focus in y defocus in x, field doubled and move closer from dipole

        /*remollDipoleField *dipolePosa = new remollDipoleField(G4ThreeVector(0.0, 0.0, 105.0*cm), G4ThreeVector(2.0*m, 2.0*m, 5.0*cm), 1.9944*tesla);
        remollDipoleField *dipoleNega = new remollDipoleField(G4ThreeVector(0.0, -0.5*m, 225.4*cm), G4ThreeVector(20.0*cm, 20.0*cm, 5.0*cm), -1.9944*tesla);
        remollDipoleField *dipoleNegb = new remollDipoleField(G4ThreeVector(0.0, -0.5*m, 474.6*cm), G4ThreeVector(20.0*cm, 20.0*cm, 5.0*cm), -1.9944*tesla);
        remollDipoleField *dipolePosb = new remollDipoleField(G4ThreeVector(0.0, 0.0, 595.0*cm), G4ThreeVector(20.0*cm, 20.0*cm, 5.0*cm), 1.9944*tesla);*/
        
        remollDipoleField *dipolePos1 = new remollDipoleField(G4ThreeVector(0.0, 0.0, 102.5*cm), G4ThreeVector(80*cm, 100*cm, 15*cm), 0.5*tesla);//standard dipople with 0.5 T
	//remollDipoleField *dipolePos1 = new remollDipoleField(G4ThreeVector(0.0, 0.0, 102.5*cm), G4ThreeVector(80*cm, 100*cm, 15*cm), 0.75*tesla);
        remollDipoleField *dipoleNeg1 = new remollDipoleField(G4ThreeVector(0.0, -20.9528*cm, 172.5*cm), G4ThreeVector(80*cm, 8*cm, 15*cm), -0.5*tesla);
        remollDipoleField *dipoleNeg2 = new remollDipoleField(G4ThreeVector(0.0, -20.9528*cm, 202.5*cm), G4ThreeVector(80*cm, 8*cm, 15*cm), -0.75*tesla);
        remollDipoleField *dipolePos2 = new remollDipoleField(G4ThreeVector(0.0, 0.0, 247.7871*cm), G4ThreeVector(80*cm, 8*cm, 15*cm), 0.75*tesla);

        //Add new fields to their respective arrays
        fQuadFields[0] = quad;
        fDipoleFields[0] = dipolePos1;
        fDipoleFields[1] = dipoleNeg1;
        fDipoleFields[2] = dipoleNeg2;
        fDipoleFields[3] = dipolePos2;

        //Add new fields to be considered as part of the GlobalField
        fGlobalField -> AddNewField(quad);
        fGlobalField -> AddNewField(dipolePos1);
        fGlobalField -> AddNewField(dipoleNeg1);
        fGlobalField -> AddNewField(dipoleNeg2);
        fGlobalField -> AddNewField(dipolePos2);
	
        fGlobalFieldManager = G4TransportationManager::GetTransportationManager()->GetFieldManager();
	fGlobalFieldManager->SetDetectorField(fGlobalField);
	fGlobalFieldManager->CreateChordFinder(fGlobalField); 

	return;	
}

void remollDetectorConstruction::SetDetectorGeomFile(const G4String &str){
	fDetFileName = str;
}

remollQuadField* remollDetectorConstruction::getQuadByIndex(int i){
   assert (i >= 0 && i < __FIELDARRAYSIZE && fQuadFields[i]);
        return fQuadFields[i];
   // G4cerr << "Cannot access quadrupole at index " << i << G4endl;
}

remollDipoleField* remollDetectorConstruction::getDipoleByIndex(int i){
    assert (i >= 0 && i < __FIELDARRAYSIZE && fDipoleFields[i]);
        return fDipoleFields[i];
   // G4cerr << "Cannot access dipole at index " << i << G4endl;
}
