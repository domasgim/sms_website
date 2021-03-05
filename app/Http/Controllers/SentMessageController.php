<?php

namespace App\Http\Controllers;

use App\SentMessage;
use Illuminate\Http\Request;
use Illuminate\Support\Facades\App;

use App\Rules\PhoneNumber;

use \ZMQ;
use \ZMQContext;
use \ZMQSocket;

class SentMessageController extends Controller
{
    /**
     * Display a listing of the resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function index()
    {
        return [
            'sent_messages' => SentMessage::orderBy('created_at', 'desc')->get()
        ];
    }

    /**
     * Show the form for creating a new resource.
     *
     * @return \Illuminate\Http\Response
     */
    public function create()
    {
        //
    }

    /**
     * Store a newly created resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @return \Illuminate\Http\Response
     */
    public function store(Request $request)
    {
        $process_name = env('ZMQ_PROCESS_NAME');
        $push_address = env('ZMQ_PUSH_ADDRESS');
        $pull_address = env('ZMQ_PULL_ADDRESS');
        $response = NULL;

        $sms_data = $request->validate([
                'recipient' => ['required', new PhoneNumber],
                'content' => 'required',
                'sender' => 'required'
        ]);

        $this->zmq_push($push_address, $sms_data);
        $return_code = $this->zmq_pull($pull_address, $process_name);

        if ($return_code == 10) 
        {
            $response_message = 'Message sent!';
            $response = SentMessage::create($sms_data);
        } else if ($return_code == -10) 
        {
            $response_message = 'Error sending message through modem.';
        } else if ($return_code == -11)
        {
            $response_message = 'SMS sender not running.';
        } else 
        {
            $response_message = 'Something went wrong.';
        }

        return [
            'response' => $response,
            'message' => $response_message
        ];
    }

    /**
     * Display the specified resource.
     *
     * @param  \App\SentMessage  $sentMessage
     * @return \Illuminate\Http\Response
     */
    public function show(SentMessage $sentMessage)
    {
        //
    }

    /**
     * Show the form for editing the specified resource.
     *
     * @param  \App\SentMessage  $sentMessage
     * @return \Illuminate\Http\Response
     */
    public function edit(SentMessage $sentMessage)
    {
        //
    }

    /**
     * Update the specified resource in storage.
     *
     * @param  \Illuminate\Http\Request  $request
     * @param  \App\SentMessage  $sentMessage
     * @return \Illuminate\Http\Response
     */
    public function update(Request $request, SentMessage $sentMessage)
    {
        //
    }

    /**
     * Remove the specified resource from storage.
     *
     * @param  \App\SentMessage  $sentMessage
     * @return \Illuminate\Http\Response
     */
    public function destroy(SentMessage $sentMessage)
    {
        //
    }

    /**
     * Listen for a ZMQ response in a given address
     */
    public function zmq_pull($address, $processName)
    {
        $context = new ZMQContext();
        $socket = new ZMQSocket($context, ZMQ::SOCKET_PULL);
        $socket->setSockOpt(ZMQ::SOCKOPT_RCVTIMEO, 10000); /* Timeout for receiving messages in ms */
        $socket->connect($address);

        if($this->processExists($processName)) {/* First, check if our SMS receiver is running */
            $return_code = $socket->recv();     /* Listen for a return */
        } else { 
            $return_code = -11;                 /* SMS receiver is not running */
        }

        $socket->disconnect($address);

        return (int) $return_code;
    }
    
    /**
     * Push a message through ZMQ to a specified address
     */
    public function zmq_push($address, $message)
    {
        $context = new ZMQContext();
        $socket = new ZMQSocket($context, ZMQ::SOCKET_PUSH);
        $socket->setSockOpt(ZMQ::SOCKOPT_LINGER, 5000);
        $socket->connect($address);
        $socket->send($this->formatMessage($message));

        $socket->disconnect($address);
    }

    /**
     * Prepare a message to be sent through ZMQ
     */
    public function formatMessage($message)
    {
        return is_array($message) || is_object($message) ?
            json_encode($message) : $message;
    }

    /**
     * Check if a specified process is running
     */
    function processExists($processName) {
        $exists= false;
        exec("ps -A | grep -i $processName | grep -v grep", $pids);
        if (count($pids) > 0) {
            $exists = true;
        }
        return $exists;
    }
}