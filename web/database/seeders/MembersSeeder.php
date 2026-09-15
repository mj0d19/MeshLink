<?php

namespace Database\Seeders;

use Illuminate\Database\Console\Seeds\WithoutModelEvents;
use Illuminate\Database\Seeder;
use Illuminate\Support\Facades\DB;

class MembersSeeder extends Seeder
{
    public function run(): void
    {
        DB::table('members')->insert([
            ['member_code'=>'M01','name'=>'John Smith','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M02','name'=>'Emma Johnson','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M03','name'=>'Michael Brown','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M04','name'=>'Sarah Davis','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M05','name'=>'James Wilson','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M06','name'=>'Emily Martinez','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M07','name'=>'David Anderson','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M08','name'=>'Olivia Taylor','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M09','name'=>'Daniel Thomas','created_at'=>now(),'updated_at'=>now()],
            ['member_code'=>'M10','name'=>'Sophia Moore','created_at'=>now(),'updated_at'=>now()],
        ]);
    }
}
