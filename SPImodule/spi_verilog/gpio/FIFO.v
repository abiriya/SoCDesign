module FIFO #(parameter FIFOSIZE = 16, FIFOWIDTH = 32)
(				input Read, 
				input Write, 
				input Clock, 
				input Reset, 
				input ClearOV,
				input [FIFOWIDTH-1:0] DataIn, 
				output reg [FIFOWIDTH-1:0] DataOut, 
				output Full,
				output reg OV,
				output EMPTY,
				output reg [3:0] ReadPtr,
				output reg [3:0] WritePtr,
				input address,
				input chipselect
				//output [0:6] hexOutput0, 
				//output [0:6] hexOutput2,
				//output [0:6] hexOutput3
);

reg [FIFOWIDTH-1:0] STACK [FIFOSIZE-1:0]; //Storage elements
reg [4:0] FIFO_Counter = 0;


assign Full  = (FIFO_Counter >= 5'd16 ) ? 1'b1:1'b0; // if full counter >= 16
assign EMPTY = (FIFO_Counter == 1'b0) ? 1'b1 : 1'b0; //if empty , counter = 0

//overflow bit set when writing to an already full fifo
reg Overflow;

always @(posedge Clock, posedge Reset)
begin
	if(Reset)
		begin 
			FIFO_Counter <= 0;
			DataOut <= 0; //clear output buffer
			ReadPtr <= 0; //reset readpointer
			WritePtr <= 0; // reset writeptr
		end
	else
		begin //beginning of state machine
			if(Read && !EMPTY && (address == 1'b00) && chipselect == 1'b1) //!READ because KEY2 is active-low| on a read and not empty
				begin 
					if(!OV)
						begin
							DataOut <= STACK[ReadPtr];
							ReadPtr <= ReadPtr + 1'b1;
							FIFO_Counter <= FIFO_Counter - 1'b1; //subtract 1 count on each read
						end
//					else //we should still be able to read even if the overflow bit is set
//						begin
//							DataOut <= STACK[ReadPtr];
//							ReadPtr <= ReadPtr + 1'b1;		
//							FIFO_Counter <= FIFO_Counter - 1'b1; //subtract 2 count on read when in overflow since counter  == 17
//						end
				end
			 else if(Write && !Overflow && (address == 2'b00) && chipselect == 1'b1) //what to do on a write. !Write because active-low
				 begin
					if(!Full)
						begin
							STACK[WritePtr] <= DataIn;
							WritePtr <= WritePtr + 1'b1;
							FIFO_Counter <= FIFO_Counter + 1'b1;
						end
					else
						begin
							//if writing to full FIFO, then trigger overflow
							Overflow <= 1;
						end
				 end 
			else
				Overflow <= 0;
			
		end //end of else
end //end of block


////overflow logic
//always @(negedge Clock) 
//begin
//	if (Overflow)
//		begin
//			OV <= 1;
//		end
//	else 
//	begin
//		if(ClearOV)
//		begin
//			OV <= 0;
//		end
//		else
//		begin
//			OV <= OV;
//		end
//	end
//
//end

 

endmodule



