$fn = 100;

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

projection(cut = true)
rotate([90,0,0])
kitdropper_sidewall(17);