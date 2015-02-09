$fn = 100;

drill_m3 = 2.5;

srf10_upperplate_angle = 3;
sensdist_margin_sm = 35;
sensdist_margin_me = 35;
sensdist_margin_bi = 65;

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
			translate([0,0,-0.75])
			minkowski()
			{
				cube([150,150,1.5],true);
				cylinder(r=20,h=1.5);
			}
			translate([-100,0,-3])
			cylinder(r=30,h=5);

			translate([0,20,-2])
			minkowski()
			{
				cube([30,72,1.5],true);
				cylinder(r=16,h=3);
			}

			translate([-90,45,-4.5])	//Löcher
			cylinder(h=9,r=1.5);
			translate([90,45,-4.5])
			cylinder(h=9,r=1.5);
			translate([90,-45,-4.5])
			cylinder(h=9,r=1.5);
			translate([-90,-45,-4.5])
			cylinder(h=9,r=1.5);

			translate([20,0,0])
			{
				rotate([0,0,270])
				{
					translate([46,0,-3.5])    //Löcher PCB
					cylinder(h=7,r=drill_m3/2);
					translate([46,-36,-3.5])
					cylinder(h=7,r=drill_m3/2);
					translate([-42,36,-3.5])
					cylinder(h=7,r=drill_m3/2);
				}
			}
			
			translate([-45,23.749,-3.5])    //Löcher CMUcam
			cylinder(h=7,r=drill_m3/2);	
			translate([-45,-23.749,-3.5])
			cylinder(h=7,r=drill_m3/2);			

			translate([75,0,-1]) //Aussparung Kitdropper
			minkowski()
			{
				cube([60,23,2],true);
				cylinder(h=2,r=5);
			}

			translate([-sensdist_margin_me,0,sens_heighth])
			{
				ir_mounting_hole();
			}
			*translate([sensdist_margin_me,0,sens_heighth])
			{
				ir_mounting_hole();
			}
		
			translate([-sensdist_margin_sm,-sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([-sensdist_margin_sm,sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_sm,-sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_sm,sensdist_margin_bi,sens_heighth])
			{
				ir_mounting_hole();
			}
			translate([sensdist_margin_bi,sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([sensdist_margin_bi,-sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([-sensdist_margin_bi,sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}
			translate([-sensdist_margin_bi,-sensdist_margin_sm,sens_heighth])
			{
				rotate([0,0,90])
				ir_mounting_hole();
			}

			rotate([90,0,90-srf10_upperplate_angle])
			translate([60,0,-75])
			srf10_mounting_hole();

			rotate([90,0,90+srf10_upperplate_angle]) 
			translate([-60,0,-75])
			srf10_mounting_hole();
		}
	}