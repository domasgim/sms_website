<?php

namespace App\Http\Controllers;

use Illuminate\Http\Request;
use App\Model\ReceiveMessageJson;
use DB;

class ReceiveJsonController extends Controller
{
    public function insert(Request $request)
    {
        $sender = $request->input('sender_number');
        $recipient = $request->input('recipient_number');
        $content = $request->input('message');

        echo($sender);
        echo($recipient);
        echo($content);
        
        DB::table('received_messages')->insert([
            'sender' => $sender,
            'recipient' => $recipient,
            'content' => $content,
            'read' => 0
        ]);
    }
}
