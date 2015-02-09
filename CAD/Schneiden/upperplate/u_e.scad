$fn = 100;

drill_m3 = 2.5;

upperplate_thickness = 4;

module ir_mounting_hole()
{
	cube([3.15,46.15,20],true);
}

module srf10_mounting_hole()
{
	cube([34.15,10,3.15],true);
}

projection(cut = true)
	color("white")
	{
		difference()
		{
			//roundCornersCube(190,190,3,20);
			translate([0,0,-0.75])
			minkowski()
			{
				//Breite: 150 + 2*20
				cube([150,150,upperplate_thickness/2],true);
				cylinder(r=20,h=1.5);
			}

			translate([-90,45,-4.5])	//Löcher
			cylinder(h=9,r=1.5);
			translate([90,45,-4.5])
			cylinder(h=9,r=1.5);
			translate([90,-45,-4.5])
			cylinder(h=9,r=1.5);
			translate([-90,-45,-4.5])
			cylinder(h=9,r=1.5);

			translate([-52.5,-44,-4.5]) //Löcher (LCD)
			cylinder(h=9,r=1.5);
			translate([12.5,-44,-4.5])
			cylinder(h=9,r=1.5);
			translate([-52.5,44,-4.5])
			cylinder(h=9,r=1.5);
			translate([12.5,44,-4.5])
			cylinder(h=9,r=1.5);

			translate([-70,-62,-4.5]) //Löcher Wärmesensoren
			cylinder(h=7,r=drill_m3/2);
			translate([70,-62,-4.5])
			cylinder(h=7,r=drill_m3/2);
			translate([70,62,-4.5])
			cylinder(h=7,r=drill_m3/2);
			translate([-70,62,-4.5])
			cylinder(h=7,r=drill_m3/2);

			translate([-45,85,-4.5]) //Löcher UIunit
			cylinder(h=9,r=1.5);
			translate([5,85,-4.5])
			cylinder(h=9,r=1.5);
			translate([-2,85,-4.5]) //LEDs
			cylinder(h=9,r=2.5);
			translate([-38,85,-4.5])
			cylinder(h=9,r=2.5);

			translate([-5,63,-2.5])
			cylinder(h=5,r=3); //Inkrementalgeber

			translate([-35,63,-2.5])
			cylinder(h=5,r=3); //Taster

			translate([-20,-63,0])
			cube([19.2,12.5,5],true); //Schalter (Power)

			translate([86,0,0]) //Aussparung Kitdropper
			cube([18.1,33.1,4],true);
		}
	}