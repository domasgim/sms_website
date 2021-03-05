<?php

return [

    'connections' => [

        'default' => [
            /*
            |--------------------------------------------------------------------------
            | ZeroMQ protocol
            |--------------------------------------------------------------------------
            | Transport protocol that'll be used to
            | establish ZeroMQ connections.
            |
            */

            'protocol' => 'tcp',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ host
            |--------------------------------------------------------------------------
            | IP or hostname of peer with which ZeroMQ connection
            | should be established.
            |
            */

            'host' => 'localhost',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ port
            |--------------------------------------------------------------------------
            | Port to which ZeroMQ connection should
            | be established.
            */

            'port' => 28332,
        ],

        '5555' => [
            /*
            |--------------------------------------------------------------------------
            | ZeroMQ protocol
            |--------------------------------------------------------------------------
            | Transport protocol that'll be used to
            | establish ZeroMQ connections.
            |
            */

            'protocol' => 'tcp',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ host
            |--------------------------------------------------------------------------
            | IP or hostname of peer with which ZeroMQ connection
            | should be established.
            |
            */

            'host' => 'localhost',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ port
            |--------------------------------------------------------------------------
            | Port to which ZeroMQ connection should
            | be established.
            */

            'port' => 5555,
        ],

        '5556' => [
            /*
            |--------------------------------------------------------------------------
            | ZeroMQ protocol
            |--------------------------------------------------------------------------
            | Transport protocol that'll be used to
            | establish ZeroMQ connections.
            |
            */

            'protocol' => 'tcp',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ host
            |--------------------------------------------------------------------------
            | IP or hostname of peer with which ZeroMQ connection
            | should be established.
            |
            */

            'host' => 'localhost',

            /*
            |--------------------------------------------------------------------------
            | ZeroMQ port
            |--------------------------------------------------------------------------
            | Port to which ZeroMQ connection should
            | be established.
            */

            'port' => 5556,
        ],
    ],
];
