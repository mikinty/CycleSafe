package com.example.CycleSafe;

import android.os.Bundle;
import android.support.annotation.NonNull;
import android.view.View;


import android.support.design.widget.BottomNavigationView;
import android.support.v7.app.AppCompatActivity;
import android.view.MenuItem;
import android.widget.TextView;

import android.content.Intent;
import android.util.Log;
import android.widget.Toast;

//new

import java.io.IOException;
import java.io.OutputStream;
import java.util.UUID;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;

import java.lang.reflect.Method;

import android.widget.ToggleButton;
import android.widget.Switch;



public class DashboardFragment extends AppCompatActivity {

    private TextView mTextMessage;

//    private BottomNavigationView.OnNavigationItemSelectedListener mOnNavigationItemSelectedListener
//            = new BottomNavigationView.OnNavigationItemSelectedListener() {
//
//        @Override
//        public boolean onNavigationItemSelected(@NonNull MenuItem item) {
//            switch (item.getItemId()) {
//                case R.id.navigation_home:
//                    mTextMessage.setText(R.string.title_home);
//                    return true;
//                case R.id.navigation_dashboard:
//                    mTextMessage.setText(R.string.title_dashboard);
//                    return true;
//                case R.id.navigation_notifications:
//                    mTextMessage.setText(R.string.title_notifications);
//                    return true;
//            }
//            return false;
//        }
//    };


    private static final String TAG = "DashActivity";


    //new
    BluetoothSocket mmSocket;
    BluetoothDevice mmDevice = null;

    final byte delimiter = 33;
    int readBufferPosition = 0;
    //new

    //new
    ToggleButton simpleToggleButton;
    Switch brakeLightsSet;
    Switch leftTurnSignalSet, rightTurnSignalSet;
    Switch leftProxSet, rightProxSet;
    Switch leftFrontProxSet, rightFrontProxSet;
    Switch leftSoundSet, rightSoundSet;
    Switch leftHapticSet, rightHapticSet;
    Switch blTEST, fullSysTEST ;


    public void writeData(BluetoothSocket mmsock, int data)
    {
        OutputStream tmpOut = null;

        // Get the BluetoothSocket input and output streams
        try {
            tmpOut = mmsock.getOutputStream();
        } catch (IOException e) {
            Log.e(TAG, "temp sockets not created", e);
        }

        try {
            tmpOut.write(data);



        } catch (IOException e) {
            Log.e(TAG, "Exception during write", e);
        }

    }
    public void connectBT(BluetoothDevice device) {
        mmDevice = device;
        BluetoothSocket tmp = null;
        UUID SerialPortServiceClass_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");


        // Get a BluetoothSocket for a connection with the
        // given BluetoothDevice
        try {
            if ( true ) {
                Method method;

                method = device.getClass().getMethod("createRfcommSocket", new Class[] { int.class } );
                //Toast.makeText(this, "finish1", Toast.LENGTH_SHORT).show();

                tmp = (BluetoothSocket) method.invoke(device, 1);
                Toast.makeText(this, "finish2", Toast.LENGTH_SHORT).show();

            }
            else {
                tmp = device.createRfcommSocketToServiceRecord( SerialPortServiceClass_UUID );
            }
        } catch (Exception e) {
            Log.e(TAG, "create() failed", e);
        }
        mmSocket = tmp;


        try {
            // This is a blocking call and will only return on a
            // successful connection or an exception

            tmp.connect();
            mmSocket = tmp;
        } catch (IOException e)
        {
            Toast.makeText(this, "finish2", Toast.LENGTH_SHORT).show();

        }
        //Data(tmp, 000);



    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.fragment_dashboard);
        BottomNavigationView navigation = (BottomNavigationView) findViewById(R.id.navigation);
        navigation.setOnNavigationItemSelectedListener(navListner);

       // brakeLightsSet = (Switch) findViewById(R.id.bLights);
        leftTurnSignalSet = (Switch) findViewById(R.id.lSignal);
        rightTurnSignalSet = (Switch) findViewById(R.id.rSignal);
        leftProxSet = (Switch) findViewById(R.id.lProx);
        rightProxSet = (Switch) findViewById(R.id.rProx);
//        leftFrontProxSet = (Switch) findViewById(R.id.lFProx);
//        rightFrontProxSet = (Switch) findViewById(R.id.rFProx);
        leftSoundSet = (Switch) findViewById(R.id.lSound);
        rightSoundSet = (Switch) findViewById(R.id.rSound);
        leftHapticSet = (Switch) findViewById(R.id.lHaptic);
        rightHapticSet = (Switch) findViewById(R.id.rHaptic);
//        blTEST= (Switch) findViewById(R.id.brakeTEST);
//        fullSysTEST = (Switch) findViewById(R.id.fullTEST);


//        brakeLightsSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (brakeLightsSet.isChecked())
//                {
//                    writeData(mmSocket, 111);
//
//                    Toast.makeText(getApplicationContext(), "SENT 111", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 110);
//
//                    Toast.makeText(getApplicationContext(), "SENT 110", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });

        leftTurnSignalSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (leftTurnSignalSet.isChecked())
                {
                    writeData(mmSocket, 121);

                    Toast.makeText(getApplicationContext(), "SENT 121", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 120);

                    Toast.makeText(getApplicationContext(), "SENT 120", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        rightTurnSignalSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (rightTurnSignalSet.isChecked())
                {
                    writeData(mmSocket, 131);

                    Toast.makeText(getApplicationContext(), "SENT 131", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 130);

                    Toast.makeText(getApplicationContext(), "SENT 130", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        leftProxSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (leftProxSet.isChecked())
                {
                    writeData(mmSocket, 141);

                    Toast.makeText(getApplicationContext(), "SENT 141", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 140);

                    Toast.makeText(getApplicationContext(), "SENT 140", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        rightProxSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (rightProxSet.isChecked())
                {
                    writeData(mmSocket, 151);

                    Toast.makeText(getApplicationContext(), "SENT 151", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 150);

                    Toast.makeText(getApplicationContext(), "SENT 150", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

//        leftFrontProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (leftFrontProxSet.isChecked())
//                {
//                    writeData(mmSocket, 161);
//
//                    Toast.makeText(getApplicationContext(), "SENT 161", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 160);
//
//                    Toast.makeText(getApplicationContext(), "SENT 160", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        rightFrontProxSet.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (rightFrontProxSet.isChecked())
//                {
//                    writeData(mmSocket, 171);
//
//                    Toast.makeText(getApplicationContext(), "SENT 171", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 170);
//
//                    Toast.makeText(getApplicationContext(), "SENT 170", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });

        leftSoundSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (leftSoundSet.isChecked())
                {
                    writeData(mmSocket, 181);

                    Toast.makeText(getApplicationContext(), "SENT 181", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 180);

                    Toast.makeText(getApplicationContext(), "SENT 180", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        rightSoundSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (rightSoundSet.isChecked())
                {
                    writeData(mmSocket, 191);

                    Toast.makeText(getApplicationContext(), "SENT 191", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 190);

                    Toast.makeText(getApplicationContext(), "SENT 190", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        leftHapticSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (leftHapticSet.isChecked())
                {
                    writeData(mmSocket, 201);

                    Toast.makeText(getApplicationContext(), "SENT 201", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 200);

                    Toast.makeText(getApplicationContext(), "SENT 200", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

        rightHapticSet.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                if (rightHapticSet.isChecked())
                {
                    writeData(mmSocket, 211);

                    Toast.makeText(getApplicationContext(), "SENT 211", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
                else
                {
                    writeData(mmSocket, 210);

                    Toast.makeText(getApplicationContext(), "SENT 210", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
                }
            }
        });

//        blTEST.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (blTEST.isChecked())
//                {
//                    writeData(mmSocket, 241);
//
//                    Toast.makeText(getApplicationContext(), "SENT 241", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 240);
//
//                    Toast.makeText(getApplicationContext(), "SENT 240", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });
//
//        fullSysTEST.setOnClickListener(new View.OnClickListener() {
//            @Override
//            public void onClick(View view) {
//                if (fullSysTEST.isChecked())
//                {
//                    writeData(mmSocket, 243);
//
//                    Toast.makeText(getApplicationContext(), "SENT 243", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//                else
//                {
//                    writeData(mmSocket, 0);
//
//                    Toast.makeText(getApplicationContext(), "SENT 0", Toast.LENGTH_SHORT).show(); // display the current state of toggle button's
//                }
//            }
//        });








        BluetoothAdapter mBluetoothAdapter = BluetoothAdapter.getDefaultAdapter();

        BluetoothDevice ddevice =  mBluetoothAdapter.getRemoteDevice("B8:27:EB:7A:E0:15");
        //Toast.makeText(this, "here", Toast.LENGTH_SHORT).show();

        connectBT(ddevice);
        Toast.makeText(this, "finish", Toast.LENGTH_SHORT).show();




    }

    private BottomNavigationView.OnNavigationItemSelectedListener navListner =
            new BottomNavigationView.OnNavigationItemSelectedListener() {
                @Override
                public boolean onNavigationItemSelected(@NonNull MenuItem menuItem) {
                    //Fragment selecFrag = null;



                    switch(menuItem.getItemId())
                    {
                        case R.id.navigation_dashboard:
//                            Intent intent = new Intent(DashboardFragment.this, DashboardFragment.class);
//                            startActivity(intent);
                            break;
                        case R.id.navigation_home:
                            Intent intent = new Intent(DashboardFragment.this, MainActivity.class);
                            startActivity(intent);
                            break;


                    }
                    //getSupportFragmentManager().beginTransaction().replace(R.id.fragment_container, selecFrag).commit();

                    return true;
                }
            };



}


