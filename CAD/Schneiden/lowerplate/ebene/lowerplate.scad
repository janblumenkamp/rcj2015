$fn = 100;

drill_m3 = 2.5;

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
		translate([0,0,-1.4])
		minkowski()
		{
			cube([180, 125,3],true);
			cylinder(r=5,h=3);
		}

		rotate([90,0,0])
		{
			translate([44.39,4,37.9])
			{
				for(j = [0:1])
				{
					for ( i = [0:2] )
					{
						translate([31.24*j,0.8,19.05-(i*6.35*3)])
						{
							rotate([90,0,0])
							{
								cylinder(h=9,r=drill_m3/2);
							}
						}
					}
				}
			}
			translate([44.39,4,-37.9])
			{
				for(j = [0:1])
				{
					for ( i = [0:2] )
					{
						translate([31.24*j,0.8,19.05-(i*6.35*3)])
						{
							rotate([90,0,0])
							{
								cylinder(h=9,r=drill_m3/2);
							}
						}
					}
				}
			}
			translate([-44.39-31.24,4,37.9])
			{
				for(j = [0:1])
				{
					for ( i = [0:2] )
					{
						translate([31.24*j,0.8,19.05-(i*6.35*3)])
						{
							rotate([90,0,0])
							{
								cylinder(h=9,r=drill_m3/2);
							}
						}
					}
				}
			}
			translate([-44.39-31.24,4,-37.9])
			{
				for(j = [0:1])
				{
					for ( i = [0:2] )
					{
						translate([31.24*j,0.8,19.05-(i*6.35*3)])
						{
							rotate([90,0,0])
							{
								cylinder(h=9,r=drill_m3/2);
							}
						}
					}
				}
			}
		}

		translate([-90,45,-4.5])	//Löcher für 2. Etage
		cylinder(h=9,r=drill_m3/2);
		translate([90,45,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([90,-45,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([-90,-45,-4.5])
		cylinder(h=9,r=drill_m3/2);
			
		translate([-72.5,0,0])
		{
			translate([55,32.5,-3.5])    //Löcher Power Unit
			cylinder(h=7,r=drill_m3/2);
			translate([90,32.5,-3.5])
			cylinder(h=7,r=drill_m3/2);
			translate([55,-32.5,-3.5])
			cylinder(h=7,r=drill_m3/2);
			translate([90,-32.5,-3.5])
			cylinder(h=7,r=drill_m3/2);
		}			

		translate([4.5,46,-4.5])	//UM6 Loch
		cylinder(h=9,r=drill_m3/2);

		translate([0,-52,-4.5])	//LED Treiber Loch
		cylinder(h=9,r=drill_m3/2);

		translate([-65,0,-4.5])	//Loch Groundsens
		cylinder(h=9,r=drill_m3/2);

		translate([86,21.5,-4.5])	//Löcher Kitdropper
		cylinder(h=9,r=drill_m3/2);
		translate([86,-21.5,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([50,21.5,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([50,-21.5,-4.5])
		cylinder(h=9,r=drill_m3/2);

		translate([88,16.5,-4.5])	//Löcher IRdist unten
		cylinder(h=9,r=drill_m3/2);
		translate([88,53.5,-4.5])
		cylinder(h=9,r=drill_m3/2);

		translate([-92,-25,-4.5]) //Löcher Untergrundsensor Option 1
		cylinder(h=9,r=drill_m3/2);
		translate([-92,-55,-4.5])
		cylinder(h=9,r=drill_m3/2);		
		translate([-82,-25,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([-82,-55,-4.5])
		cylinder(h=9,r=drill_m3/2);		

		translate([-92,25,-4.5])  //Löcher Untergrundsensor Option 2
		cylinder(h=9,r=drill_m3/2);
		translate([-92,55,-4.5])
		cylinder(h=9,r=drill_m3/2);		
		translate([-82,25,-4.5])
		cylinder(h=9,r=drill_m3/2);
		translate([-82,55,-4.5])
		cylinder(h=9,r=drill_m3/2);		

		hull() //Kabeldurchgänge
		{
			translate([-30,0,-4.5]) 
			cylinder(h=9,r=10);
			translate([-30,-25,-4.5])
			cylinder(h=9,r=10);
		}
		hull()
		{
			translate([30,0,-4.5])
			cylinder(h=9,r=10);
			translate([30,-25,-4.5])
			cylinder(h=9,r=10);
		}

		translate([-85,0,0]) //SRF10 Loch
		rotate([270,0,90])
		srf10_mounting_hole();

		translate([81,0,3]) //IR Dist Loch
		ir_mounting_hole();
	}
}
