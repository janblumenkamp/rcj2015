kitdropper(27,17,16);

module kitdropper_sidewall(length)
{
	color("white")
	{
		difference()
		{
			translate([18,0,5.25])  //Große Seitenabdeckung
			cube([length+1+36,3,81.5],true);
		
			translate([27.5,0,21]) //Aussparung große Fläche
			cube([37,4,70],true);
		
			translate([27,0,13]) //Aussparung Befestigung Servohalter
			cube([25,4,60],true);
		
			translate([length/2-1+0.5,0,6]) //Aussparung Rückwand
			cube([4,4,30],true);
		
			translate([-8,0,6]) //Aussparung vorderwand
			cube([4,4,30],true);
		}
	}
}

module kitdropper_frontbackwall(width, length)
{
	difference() //Vorderwand
	{
		translate([-(length/2-1),0,8.75]) 
		cube([3,width+6,74.5],true);
		
		translate([-9.5,13.5,-29])
		cube([4,5,20]);
		translate([-9.5,-18.5,-29])
		cube([4,5,20]);

		translate([-9.5,13.5,21])
		cube([4,5,30]);
		translate([-9.5,-18.5,21])
		cube([4,5,30]);
	}
}

module kitdropper(width, length, dropperpos)
{
	color("white")
	{
		translate([0,-(width/2+1.5),0])
		kitdropper_sidewall(length);
		translate([0,-(width/2-1.5),8.75])  //Schacht schmaler
		cube([length-5.8,3,81.5-7],true);
		difference() //Befestigungsleiste
		{
			translate([18,length/2+12,-34])
			cube([length+1+36,8,3],true);
			
			translate([0,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
			translate([36,length/2+13,-37])
			cylinder(h = 5, r = 1.5);
		}
		
		translate([0,width/2+1.5,0])
		kitdropper_sidewall(length);
		translate([0,width/2-1.5,8.75])  //Schacht schmaler
		cube([length-5.8,3,81.5-7],true);
		difference()
		{
			translate([18,-(length/2+12),-34])
			cube([length+1+36,8,3],true);
			
			translate([0,-(length/2+13),-37])
			cylinder(h = 5, r = 1.5);
			translate([36,-(length/2+13),-37])
			cylinder(h = 5, r = 1.5);
		}
		
		kitdropper_frontbackwall(width, length);
		translate([length-2,0,0])
		kitdropper_frontbackwall(width, length);
	}
	/*color("white")
	{
		translate([dropperpos+5,0,-32.95])
		difference()
		{
			translate([-15.5/2+1.5,0,0])
			cube([length+15+15.5,width-1,5],true);
			
			hull()
			{
				translate([length/2+2.5,-(width/2-3.5),-5])
				cylinder(h=10,r=2);
				translate([length/2+2.5,width/2-3.5,-5])
				cylinder(h=10,r=2);
			}
			translate([-21,0,0])
			cube([13,22,10],true);
		}
	}*/
	
	color("white")
	difference()
	{
		translate([27,0,-15.5])
		cube([36,33,3],true);
	
		hull() //Servobefestigung 1
		{
			translate([13,10,-18])
			cylinder(h=5,r=1.5);
			translate([13,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([27,0,-16.5]) //Aussparung Servo
		minkowski()
		{
			cube([18,22,2],true);
			cylinder(h=2,r=2);
		}
		
		hull() //Servobefestigung 2
		{
			translate([41,10,-18])
			cylinder(h=5,r=1.5);
			translate([41,-10,-18])
			cylinder(h=5,r=1.5);
		}
		
		translate([7.5,13.5,-17.5]) //Aussparungen an den vier Ecken
		cube([7,4,4]);
	
		translate([7.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,-17.5,-17.5])
		cube([7,4,4]);
	
		translate([39.5,13.5,-17.5])
		cube([7,4,4]);
	}
	translate([10,7,-0.5])
	rotate([0,180,0])
	servo_micro();
	
	/*translate([0,0,-32.45])
	for(i = [0:11])
	{
		color("red")
		translate([0,0,i*6.2])
		cube([length-6,width-7,6],true);
	}*/

}

module rescuekit()
{
	color("dimgray")
	cylinder(h=5.5,r=5.25);
	translate([0,0,-0.2])
	color("silver")
	cylinder(h=6.1,r=3.5);

	difference()
	{
		translate([-4.5,-10,-0.25])
		cube([9,15,6]);
		translate([0,0,-2])
		cylinder(h=10,r=4.8);
	}
}
